// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime;

import android.app.Activity;
import android.content.res.AssetManager;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;

/**
 * This internal class parses manifest.json of current app.
 */
public class XWalkManifestReader {
    private final static String TAG = "XWalkManifestReader";
    private final static String ASSETS_FILE_PATH = "file:///android_asset/";

    private Activity mActivity;

    public XWalkManifestReader(Activity activity) {
        mActivity = activity;
    }

    public String read(String manifestPath) {
        manifestPath = deletePrefix(manifestPath, ASSETS_FILE_PATH);

        String manifestString = null;
        try {
            manifestString = getAssetsFileContent(mActivity.getAssets(), manifestPath);
        } catch (IOException e) {
            throw new RuntimeException("Failed to read manifest.json", e);
        }
        return manifestString;
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

    private String deletePrefix(String str, String prefix) {
        String result = str;
        if (str != null && prefix != null && str.startsWith(prefix)) {
            result = str.substring(prefix.length());
        }
        return result;
    }
}
