// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.extension;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;

import org.xwalk.core.internal.XWalkExtensionInternal;
import org.xwalk.core.internal.extension.api.launchscreen.LaunchScreenExtension;
import org.xwalk.core.internal.extension.api.wifidirect.WifiDirect;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.content.res.Resources.NotFoundException;
import android.util.Log;

public class BuiltinXWalkExtensions {
    private static final String TAG = "BuiltinXWalkExtension";
    private static HashMap<String, XWalkExtensionInternal> sBuiltinExtensions =
            new HashMap<String, XWalkExtensionInternal>();

    public static void load(Context context) {
        // Create all built-in extension instances here.

        {
            String jsApiContent = "";
            try {
                jsApiContent = getExtensionJSFileContent(
                        context, LaunchScreenExtension.JS_API_PATH, true);
                sBuiltinExtensions.put(LaunchScreenExtension.JS_API_PATH,
                        new LaunchScreenExtension(jsApiContent, context.getApplicationContext()));
            } catch (IOException e) {
                Log.w(TAG, "Failed to read JS API file: " + LaunchScreenExtension.JS_API_PATH);
            }
        }

        if (context instanceof Activity) {
            String jsApiContent = "";
            try {
                jsApiContent = getExtensionJSFileContent(
                        context, WifiDirect.JS_API_PATH, true);

                sBuiltinExtensions.put(WifiDirect.JS_API_PATH,
                        new WifiDirect(jsApiContent, (Activity) context));
            } catch(IOException e) {
                Log.w(TAG, "Failed to read JS API file: " + WifiDirect.JS_API_PATH);
            }
        }
    }


    private static String getExtensionJSFileContent(Context context, String fileName, boolean fromRaw)
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
}
