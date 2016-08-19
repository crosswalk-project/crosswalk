// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extension;

import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.content.res.Resources.NotFoundException;
import android.os.Bundle;
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

import org.xwalk.core.XWalkExternalExtensionManager;
import org.xwalk.core.XWalkNativeExtensionLoader;
import org.xwalk.core.XWalkView;

/**
 * This internal class acts as a manager to manage external extensions.
 * It provides impl for org.xwalk.core.XWalkExternalExtensionManager,
 * and will be created automatically by XWalkView constructor.
 * Any XWalkView embedder could call XWalkView.getExtensionManager() to get this manager instance.
 */
public class XWalkExternalExtensionManagerImpl extends XWalkExternalExtensionManager
        implements XWalkExtensionContextClient {
    private static final String TAG = "XWalkExternalExtensionManagerImpl";

    private final XWalkView mXWalkView;
    private final Context mContext;

    private final HashMap<String, XWalkExternalExtensionBridge> mExtensions = new HashMap<String, XWalkExternalExtensionBridge>();
    // This variable is to set whether to load external extensions. The default is true.
    private boolean mLoadExternalExtensions;
    private final XWalkNativeExtensionLoader mNativeExtensionLoader;

    public XWalkExternalExtensionManagerImpl(XWalkView view) {
        super(view);

        mXWalkView = view;

        if (getBridge() == null) {
            Log.e(TAG, "Cannot load external extensions due to old version of runtime library");
            mContext = null;
            mLoadExternalExtensions = false;
            mNativeExtensionLoader = null;
            return;
        }

        mContext = getViewContext();
        mLoadExternalExtensions = true;
        mNativeExtensionLoader = new XWalkNativeExtensionLoader();

        loadNativeExtensions();
    }

    // Start to override XWalkExtensionContextClient api.
    @Override
    public void registerExtension(XWalkExternalExtension extension) {
        if (mExtensions.get(extension.getExtensionName()) != null) {
            Log.e(TAG, extension.getExtensionName() + "is already registered!");
            return;
        }

        XWalkExternalExtensionBridge bridge = XWalkExternalExtensionBridgeFactory.createInstance(extension);
        mExtensions.put(extension.getExtensionName(), bridge);
    }

    @Override
    public void unregisterExtension(String name) {
        XWalkExternalExtensionBridge bridge = mExtensions.get(name);
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
        if (mContext instanceof Activity) {
            return (Activity) mContext;
        }
        return null;
    }

    // Instead of calling startActivityForResult() on mActivity directly,
    // call on XWalkView so that XWalkView's embedder could override this behavior
    // to do additional job.
    @Override
    public void startActivityForResult(Intent intent, int requestCode, Bundle options) {
        throw new ActivityNotFoundException("This method is no longer supported");
    }


    // Start to override XWalkExternalExtensionManager api.
    // Load one Java external extension by its folder path under assets/xwalk-extensions,
    // the extension folder structure should be like:
    // ExtensionA
    //     ExtensionA.json
    //     ExtensionA.js(Optional)
    @Override
    public void loadExtension(String extensionPath) {
        if (!mLoadExternalExtensions) return;

        // Read extension json file.
        int len = extensionPath.length();
        if (extensionPath.charAt(len - 1) == File.separatorChar) {
            extensionPath = extensionPath.substring(0, len - 1);
        }
        String folderName = extensionPath.substring(extensionPath.lastIndexOf(File.separatorChar) + 1);
        String jsonFile = extensionPath + File.separator + folderName + ".json";
        String jsonFileContent;
        try {
            jsonFileContent = getFileContent(mContext, jsonFile, false);
        } catch (IOException e) {
            Log.w(TAG, "Failed to read json file: " + jsonFile);
            return;
        }

        try {
            JSONObject jsonObject = new JSONObject(jsonFileContent);
            String name = jsonObject.getString("name");
            String className =  jsonObject.getString("class");
            String jsApiFile = jsonObject.optString("jsapi");

            // Load the content of the JavaScript file.
            if (jsApiFile != null && jsApiFile.length() != 0) {
                jsApiFile = extensionPath + File.separator + jsApiFile;
            }

            String jsApi = null;
            if (jsApiFile != null && jsApiFile.length() != 0) {
                try {
                    jsApi = getFileContent(mContext, jsApiFile, false);
                } catch (IOException e) {
                    Log.w(TAG, "Failed to read the file " + jsApiFile);
                    return;
                }
            }

            if (name != null && className != null) {
                Log.i(TAG, "createExternalExtension: name: " + name + " className: " + className);
                createExternalExtension(name, className, jsApi, this);
            }
        } catch (JSONException e) {
            Log.w(TAG, "Failed to parse json file: " + jsonFile);
        }
    }

    @Override
    public void postMessage(XWalkExternalExtension extension, int instanceID, String message) {
        XWalkExternalExtensionBridge bridge = mExtensions.get(extension.getExtensionName());
        if (bridge != null) bridge.postMessage(instanceID, message);
    }

    @Override
    public void postBinaryMessage(XWalkExternalExtension extension, int instanceID, byte[] message) {
        XWalkExternalExtensionBridge bridge = mExtensions.get(extension.getExtensionName());
        if (bridge != null) bridge.postBinaryMessage(instanceID, message);
    }

    @Override
    public void broadcastMessage(XWalkExternalExtension extension, String message) {
        XWalkExternalExtensionBridge bridge = mExtensions.get(extension.getExtensionName());
        if (bridge != null) bridge.broadcastMessage(message);
    }

    @Override
    public void onStart() {
        for(XWalkExternalExtensionBridge extension: mExtensions.values()) {
            extension.onStart();
        }
    }

    @Override
    public void onResume() {
        for(XWalkExternalExtensionBridge extension: mExtensions.values()) {
            extension.onResume();
        }
    }

    @Override
    public void onPause() {
        for(XWalkExternalExtensionBridge extension: mExtensions.values()) {
            extension.onPause();
        }
    }

    @Override
    public void onStop() {
        for(XWalkExternalExtensionBridge extension: mExtensions.values()) {
            extension.onStop();
        }
    }

    @Override
    public void onDestroy() {
        for(XWalkExternalExtensionBridge extension: mExtensions.values()) {
            extension.onDestroy();
        }
        mExtensions.clear();
    }

    @Override
    public void onNewIntent(Intent intent) {
        for(XWalkExternalExtensionBridge extension: mExtensions.values()) {
            extension.onNewIntent(intent);
        }
    }

    public void setAllowExternalExtensions(boolean load) {
        mLoadExternalExtensions = load;
    }

    private String getFileContent(Context context, String fileName, boolean fromRaw)
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
        Context context = extensionContext.getContext();
        try {
            Class<?> clazz = context.getClassLoader().loadClass(className);
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

    // Load all C++ external extensions under native library folder.
    private void loadNativeExtensions() {
        // If there is a native external extension, register the path. The
        // extensions under the path will be loaded automatically when the
        // native service starts.
        String path = null;
        try {
            ApplicationInfo appInfo =
                    mContext.getPackageManager().getApplicationInfo(mContext.getPackageName(), 0);
            path = appInfo.nativeLibraryDir;
        } catch (final NameNotFoundException e) {
        }
        if (path != null && new File(path).isDirectory()) {
            mNativeExtensionLoader.registerNativeExtensionsInPath(path);
        }
    }

    private static void handleException(Exception e) {
        // TODO(yongsheng): Handle exceptions here.
        Log.e(TAG, "Error in calling methods of external extensions. " + e.toString());
        e.printStackTrace();
    }
}
