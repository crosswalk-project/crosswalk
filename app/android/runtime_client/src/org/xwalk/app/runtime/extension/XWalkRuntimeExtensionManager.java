// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app.runtime.extension;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.content.res.Resources.NotFoundException;
import android.util.Log;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.Class;
import java.util.HashMap;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import org.xwalk.core.XWalkNativeExtensionLoader;

/**
 * This internal class acts a manager to manage extensions.
 */
public class XWalkRuntimeExtensionManager implements XWalkExtensionContextClient {
    private final static String TAG = "XWalkExtensionManager";
    private final static String EXTENSION_CONFIG_FILE = "extensions-config.json";

    private final Context mContext;
    private final Activity mActivity;

    private final HashMap<String, XWalkRuntimeExtensionBridge> mExtensions = new HashMap<String, XWalkRuntimeExtensionBridge>();
    // This variable is to set whether to load external extensions. The default is true.
    private boolean mLoadExternalExtensions;
    private final XWalkNativeExtensionLoader mNativeExtensionLoader;

    public XWalkRuntimeExtensionManager(Context context, Activity activity) {
        mContext = context;
        mActivity = activity;
        mLoadExternalExtensions = true;
        mNativeExtensionLoader = new XWalkNativeExtensionLoader();
    }

    @Override
    public void registerExtension(XWalkExtensionClient extension) {
        if (mExtensions.get(extension.getExtensionName()) != null) {
            Log.e(TAG, extension.getExtensionName() + "is already registered!");
            return;
        }

        XWalkRuntimeExtensionBridge bridge = XWalkRuntimeExtensionBridgeFactory.createInstance(extension);
        mExtensions.put(extension.getExtensionName(), bridge);
    }

    @Override
    public void unregisterExtension(String name) {
        XWalkRuntimeExtensionBridge bridge = mExtensions.get(name);
        if (bridge != null) {
            mExtensions.remove(name);
            bridge.onDestroy();
        }
    }

    @Override
    public Context getContext() {
        return mContext;
    }

    @Override
    public Activity getActivity() {
        return mActivity;
    }

    @Override
    public void postMessage(XWalkExtensionClient extension, int instanceID, String message) {
        XWalkRuntimeExtensionBridge bridge = mExtensions.get(extension.getExtensionName());
        if (bridge != null) bridge.postMessage(instanceID, message);
    }

    @Override
    public void postBinaryMessage(XWalkExtensionClient extension, int instanceID, byte[] message) {
        XWalkRuntimeExtensionBridge bridge = mExtensions.get(extension.getExtensionName());
        if (bridge != null) bridge.postBinaryMessage(instanceID, message);
    }

    public void broadcastMessage(XWalkExtensionClient extension, String message) {
        XWalkRuntimeExtensionBridge bridge = mExtensions.get(extension.getExtensionName());
        if (bridge != null) bridge.broadcastMessage(message);
    }

    public void onStart() {
        for(XWalkRuntimeExtensionBridge extension: mExtensions.values()) {
            extension.onStart();
        }
    }

    public void onResume() {
        for(XWalkRuntimeExtensionBridge extension: mExtensions.values()) {
            extension.onResume();
        }
    }

    public void onPause() {
        for(XWalkRuntimeExtensionBridge extension: mExtensions.values()) {
            extension.onPause();
        }
    }

    public void onStop() {
        for(XWalkRuntimeExtensionBridge extension: mExtensions.values()) {
            extension.onStop();
        }
    }

    public void onDestroy() {
        for(XWalkRuntimeExtensionBridge extension: mExtensions.values()) {
            extension.onDestroy();
        }
        mExtensions.clear();
    }

    public void onNewIntent(Intent intent) {
        for(XWalkRuntimeExtensionBridge extension: mExtensions.values()) {
            extension.onNewIntent(intent);
        }
    }

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        for(XWalkRuntimeExtensionBridge extension: mExtensions.values()) {
            extension.onActivityResult(requestCode, resultCode, data);
        }
    }

    public void loadExtensions() {
        loadExternalExtensions();
    }

    public void setAllowExternalExtensions(boolean load) {
        mLoadExternalExtensions = load;
    }

    private void loadExternalExtensions() {
        if (!mLoadExternalExtensions) return;

        // If there is a native external extension, register the path. The
        // extensions under the path will be loaded automatically when the
        // native service starts.
        String path = null;
        try {
            ApplicationInfo appInfo =
                    mContext.getPackageManager()
                    .getApplicationInfo(mActivity.getPackageName(), 0);
            path = appInfo.nativeLibraryDir;
        } catch (final NameNotFoundException e) {
        }
        if (path != null && new File(path).isDirectory())
            mNativeExtensionLoader.registerNativeExtensionsInPath(path);

        // Read extensions-config.json and create external extensions.
        String configFileContent;
        try {
            configFileContent = getExtensionJSFileContent(mActivity, EXTENSION_CONFIG_FILE, false);
        } catch (IOException e) {
            Log.w(TAG, "Failed to read extensions-config.json");
            return;
        }

        try {
            JSONArray jsonFeatures = new JSONArray(configFileContent);
            int extensionCount = jsonFeatures.length();
            for (int i = 0; i < extensionCount; i++) {
                JSONObject jsonObject = jsonFeatures.getJSONObject(i);
                String name = jsonObject.getString("name");
                String className =  jsonObject.getString("class");
                String jsApiFile = jsonObject.optString("jsapi");

                // Load the content of the JavaScript file.
                String jsApi = null;
                if (jsApiFile != null && jsApiFile.length() != 0) {
                    try {
                        jsApi = getExtensionJSFileContent(mActivity, jsApiFile, false);
                    } catch (IOException e) {
                        Log.w(TAG, "Failed to read the file " + jsApiFile);
                        return;
                    }
                }

                if (name != null && className != null) {
                    createExternalExtension(name, className, jsApi, this);
                }
            }
        } catch (JSONException e) {
            Log.w(TAG, "Failed to parse extensions-config.json");
        }
    }

    private String getExtensionJSFileContent(Context context, String fileName, boolean fromRaw)
            throws IOException {
        String result = "";
        InputStream inputStream = null;
        try {
            if (fromRaw) {
                // If fromRaw is true, Try to find js file in res/raw first.
                // And then try to get it from assets if failed.
                Resources resource = context.getResources();
                String resName = (new File(fileName).getName().split("\\."))[0];
                int resId = resource.getIdentifier(resName, "raw", context.getPackageName());
                if (resId > 0) {
                    try {
                        inputStream = resource.openRawResource(resId);
                    } catch (NotFoundException e) {
                        Log.w(TAG, "Inputstream failed to open for R.raw." + resName +
                                   ", try to find it in assets");
                    }
                }
            }
            if (inputStream == null) {
                AssetManager assetManager = context.getAssets();
                inputStream = assetManager.open(fileName);
            }
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

    private void createExternalExtension(String name, String className, String jsApi,
            XWalkExtensionContextClient extensionContext) {
        Activity activity = extensionContext.getActivity();
        try {
            Class<?> clazz = activity.getClassLoader().loadClass(className);
            Constructor<?> constructor = clazz.getConstructor(String.class,
                    String.class, XWalkExtensionContextClient.class);
            constructor.newInstance(name, jsApi, this);
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
        e.printStackTrace();
    }
}
