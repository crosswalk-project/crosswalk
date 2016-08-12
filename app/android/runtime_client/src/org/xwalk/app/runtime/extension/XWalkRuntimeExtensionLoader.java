// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app.runtime.extension;

import android.content.Context;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.content.res.Resources.NotFoundException;
import android.util.Log;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import org.xwalk.core.XWalkExternalExtensionManager;
import org.xwalk.core.XWalkView;

/**
 * This internal class loads all external extensions under assets/xwalk-extensions.
 * Employ XWalkView.getExtensionManager() to load external extensions actually.
 */
public class XWalkRuntimeExtensionLoader {
    private static final String TAG = "XWalkExtensionLoader";
    private static final String XWALK_EXTENSIONS_FOLDER = "xwalk-extensions";

    public static void loadExtensions(XWalkView xwalkView, Context context) {
        loadExternalExtensions(xwalkView, context);
    }

    private static void loadExternalExtensions(XWalkView xwalkView, Context context) {
        AssetManager assetManager = context.getAssets();
        String[] extList;
        try {
            Log.i(TAG, "Iterate assets" + File.separator + XWALK_EXTENSIONS_FOLDER);
            extList = assetManager.list(XWALK_EXTENSIONS_FOLDER);
        } catch (IOException e) {
            Log.w(TAG, "Failed to iterate assets" + File.separator + XWALK_EXTENSIONS_FOLDER);
            return;
        }

        for (String path : extList) {
            // Load the extension.
            Log.i(TAG, "Start to load extension: " + path);
            XWalkExternalExtensionManager extensionManager = xwalkView.getExtensionManager();
            extensionManager.loadExtension(XWALK_EXTENSIONS_FOLDER + File.separator + path);
        }
    }
}
