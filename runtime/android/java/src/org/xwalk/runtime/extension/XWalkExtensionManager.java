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
import java.util.ArrayList;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.xwalk.runtime.XWalkRuntimeViewProvider;
import org.xwalk.runtime.extension.api.Device;

/**
 * This internal class acts a manager to manage extensions.
 */
public class XWalkExtensionManager {
    private final static String TAG = "XWalkExtensionManager";
    private final static String EXTENSION_CONFIG_FILE = "extensions-config.json";

    private Context mContext;
    private Activity mActivity;
    private XWalkRuntimeViewProvider mXwalkProvider;
    private XWalkExtensionContextImpl mExtensionContextImpl;

    private ArrayList<XWalkExtension> mExtensions;

    public XWalkExtensionManager(Context context, Activity activity, XWalkRuntimeViewProvider xwalkProvider) {
        mContext = context;
        mActivity = activity;
        mXwalkProvider = xwalkProvider;
        mExtensionContextImpl = new XWalkExtensionContextImpl(context, activity, this);
        mExtensions = new ArrayList<XWalkExtension>();

        // TODO(gaochun): Load extensions in a callback which is invoked by runtime
        // when the patch of extension bridge(form C++ to Java) was ready.
        loadExtensions();
    }

    public XWalkExtensionContext getExtensionContext() {
        return mExtensionContextImpl;
    }

    public void postMessage(XWalkExtension extension, String message) {
        mXwalkProvider.postMessage(extension, message);
    }

    public Object registerExtension(XWalkExtension extension) {
        mExtensions.add(extension);
        return mXwalkProvider.onExtensionRegistered(extension);
    }

    public void unregisterExtensions() {
        for(XWalkExtension extension: mExtensions) {
            mXwalkProvider.onExtensionUnregistered(extension);
        }
        mExtensions.clear();
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

    private void loadExtensions() {
        loadInternalExtensions();
        loadExternalExtensions();
    }

    private void loadInternalExtensions() {
        // Create all extension instances directly here. The internal extension will register 
        // itself and add itself to XWalkExtensionManager.mExtensions automatically.
        String jsApiContent = "";
        try {
            jsApiContent = getAssetsFileContent(mContext.getAssets(), Device.JS_API_PATH);
        } catch(IOException e) {
            Log.e(TAG, "Failed to read js API file of internal extension: Device");
        }
        new Device(jsApiContent, mExtensionContextImpl);
    }

    private void loadExternalExtensions() {
        // Read extensions-config.json and create external extensions.
        String configFileContent;
        try {
            configFileContent = getAssetsFileContent(mActivity.getAssets(), EXTENSION_CONFIG_FILE);
        } catch (IOException e) {
            Log.e(TAG, "Failed to read extensions-config.json");
            return;
        }

        try {
            JSONArray jsonFeatures = new JSONArray(configFileContent);
            int extensionCount = jsonFeatures.length();
            for (int i = 0; i < extensionCount; i++) {
                JSONObject jsonObject = jsonFeatures.getJSONObject(i);
                String name = jsonObject.getString("name");
                String className =  jsonObject.getString("class");
                String apiVersion = jsonObject.getString("version");
                String jsApi = jsonObject.getString("js_api");

                if (name != null && className != null && apiVersion != null && jsApi != null) {
                    createExternalExtension(name, className, apiVersion, jsApi, mExtensionContextImpl);
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
            inputStream.close();
        }
        return result;
    }

    private void createExternalExtension(String name, String className, String apiVersion, String jsApi, 
            XWalkExtensionContext extensionContext) {
        // TODO(gaochun): Implement external extension with bridge classes.
    }
}
