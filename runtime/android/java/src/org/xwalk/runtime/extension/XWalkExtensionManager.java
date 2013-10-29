// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.res.AssetManager;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.Class;
import java.util.ArrayList;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.xwalk.runtime.extension.api.PresentationExtension;
import org.xwalk.runtime.XWalkRuntimeViewProvider;

/**
 * This internal class acts a manager to manage extensions.
 */
public class XWalkExtensionManager {
    private final static String TAG = "XWalkExtensionManager";
    private final static String EXTENSION_CONFIG_FILE = "extensions-config.json";
    // This class name is from runtime client. Need to keep consistency with it.
    private final static String EXTENSION_CONTEXT_CLIENT_CLASS_NAME =
            "org.xwalk.app.runtime.extension.XWalkExtensionContextClient";

    private Context mContext;
    private Activity mActivity;
    private XWalkRuntimeViewProvider mXwalkProvider;
    private XWalkExtensionContextImpl mExtensionContextImpl;

    private ArrayList<XWalkExtension> mExtensions;
    // This variable is to set whether to load external extensions. The default is true.
    private boolean mLoadExternalExtensions;

    public XWalkExtensionManager(Context context, Activity activity, XWalkRuntimeViewProvider xwalkProvider) {
        mContext = context;
        mActivity = activity;
        mXwalkProvider = xwalkProvider;
        mExtensionContextImpl = new XWalkExtensionContextImpl(context, activity, this);
        mExtensions = new ArrayList<XWalkExtension>();
        mLoadExternalExtensions = true;
    }

    public XWalkExtensionContext getExtensionContext() {
        return mExtensionContextImpl;
    }

    public void postMessage(XWalkExtension extension, int instanceID, String message) {
        mXwalkProvider.postMessage(extension, instanceID, message);
    }

    public void broadcastMessage(XWalkExtension extension, String message) {
        mXwalkProvider.broadcastMessage(extension, message);
    }

    public Object registerExtension(XWalkExtension extension) {
        mExtensions.add(extension);
        return mXwalkProvider.onExtensionRegistered(extension);
    }

    public void unregisterExtension(XWalkExtension extension) {
        mXwalkProvider.onExtensionUnregistered(extension);
        mExtensions.remove(extension);
    }

    public void onResume() {
        for(XWalkExtension extension: mExtensions) {
            extension.onResume();
        }
    }

    public void onPause() {
        for(XWalkExtension extension: mExtensions) {
            extension.onPause();
        }
    }

    public void onDestroy() {
        for(XWalkExtension extension: mExtensions) {
            extension.onDestroy();
        }
    }

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        for(XWalkExtension extension: mExtensions) {
            extension.onActivityResult(requestCode, resultCode, data);
        }
    }

    public void loadExtensions() {
        loadInternalExtensions();
        loadExternalExtensions();
    }

    public void setAllowExternalExtensions(boolean load) {
        mLoadExternalExtensions = load;
    }

    private void loadInternalExtensions() {
        // Create all extension instances directly here. The internal extension will register
        // itself and add itself to XWalkExtensionManager.mExtensions automatically.
        // The following sample shows how to create an extension that named Device:
        //    String jsApiContent = "";
        //    try {
        //        jsApiContent = getAssetsFileContent(mContext.getAssets(), Device.JS_API_PATH);
        //    } catch(IOException e) {
        //        Log.e(TAG, "Failed to read js API file of internal extension: Device");
        //    }
        //    new Device(jsApiContent, mExtensionContextImpl);
        {
            String jsApiContent = "";
            try {
                jsApiContent = getAssetsFileContent(mContext.getAssets(),
                                                    PresentationExtension.JS_API_PATH);
            } catch (IOException e) {
                Log.e(TAG, "Failed to read JS API file: " + PresentationExtension.JS_API_PATH);
            }
            // Load PresentationExtension as an internal extension.
            new PresentationExtension(PresentationExtension.NAME, jsApiContent, mExtensionContextImpl);
        }
    }

    private void loadExternalExtensions() {
        if (!mLoadExternalExtensions) return;

        // Read extensions-config.json and create external extensions.
        String configFileContent;
        try {
            configFileContent = getAssetsFileContent(mActivity.getAssets(), EXTENSION_CONFIG_FILE);
        } catch (IOException e) {
            Log.e(TAG, "Failed to read extensions-config.json");
            return;
        }

        // Initialize the context for external extensions.
        XWalkExtensionContextWrapper contextWrapper =
                new XWalkExtensionContextWrapper(mExtensionContextImpl);
        Object contextClient = createExtensionContextClient(contextWrapper);

        try {
            JSONArray jsonFeatures = new JSONArray(configFileContent);
            int extensionCount = jsonFeatures.length();
            for (int i = 0; i < extensionCount; i++) {
                JSONObject jsonObject = jsonFeatures.getJSONObject(i);
                String name = jsonObject.getString("name");
                String className =  jsonObject.getString("class");
                String jsApiFile = jsonObject.getString("jsapi");

                // Load the content of the JavaScript file.
                String jsApi;
                try {
                    jsApi = getAssetsFileContent(mActivity.getAssets(), jsApiFile);
                } catch (IOException e) {
                    Log.e(TAG, "Failed to read the file " + jsApiFile);
                    return;
                }

                if (name != null && className != null && jsApi != null) {
                    createExternalExtension(name, className, jsApi, contextClient, contextWrapper);
                }
            }
        } catch (JSONException e) {
            Log.e(TAG, "Failed to parse extensions-config.json");
        }
    }

    private String getAssetsFileContent(AssetManager assetManager, String fileName) throws IOException {
        String result = "";
        InputStream inputStream = null;
        try {
            inputStream = assetManager.open(fileName);
            int size = inputStream.available();
            byte[] buffer = new byte[size];
            inputStream.read(buffer);
            result = new String(buffer);
        } finally {
            if (inputStream != null) {
                inputStream.close();
            }
        }
        return result;
    }

    private Object createExtensionContextClient(XWalkExtensionContextWrapper contextWrapper) {
        Activity activity = contextWrapper.getActivity();
        try {
            Class<?> clazz = activity.getClassLoader().loadClass(EXTENSION_CONTEXT_CLIENT_CLASS_NAME);
            Constructor<?> constructor = clazz.getConstructor(Activity.class, Object.class);
            return constructor.newInstance(activity, contextWrapper);
        } catch (ClassNotFoundException e) {
            handleException(e);
        } catch (IllegalAccessException e) {
            handleException(e);
        } catch (InstantiationException e) {
            handleException(e);
        } catch (InvocationTargetException e) {
            handleException(e);
        } catch (NoSuchMethodException e) {
            handleException(e);
        }
        return null;
    }

    private void createExternalExtension(String name, String className, String jsApi,
            Object contextClient, XWalkExtensionContextWrapper contextWrapper) {
        Activity activity = contextWrapper.getActivity();
        try {
            Class<?> clazz = activity.getClassLoader().loadClass(className);
            Constructor<?> constructor = clazz.getConstructor(String.class,
                    String.class, contextClient.getClass());
            constructor.newInstance(name, jsApi, contextClient);
        } catch (ClassNotFoundException e) {
            handleException(e);
        } catch (IllegalAccessException e) {
            handleException(e);
        } catch (InstantiationException e) {
            handleException(e);
        } catch (InvocationTargetException e) {
            handleException(e);
        } catch (NoSuchMethodException e) {
            handleException(e);
        }
    }

    private static void handleException(Exception e) {
        // TODO(yongsheng): Handle exceptions here.
        Log.e(TAG, "Error in calling methods of external extensions. " + e.toString());
    }
}
