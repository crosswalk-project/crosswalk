// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extension;

import android.os.Environment;

import java.io.File;
import java.util.ArrayList;

import org.chromium.base.CalledByNative;
import org.chromium.base.JNINamespace;

@JNINamespace("xwalk")
public class XWalkPathHelper {
    private final static String TAG = "XWalkPathHelper";

    public XWalkPathHelper() {}

    public static void initialize() {
        nativeSetDirectory("EXTERNAL", Environment.getExternalStorageDirectory().getPath());

        String names[] = {
            "ALARMS",
            "DCIM",
            "DOWNLOADS",
            "MOVIES",
            "MUSIC",
            "NOTIFICATIONS",
            "PICTURES",
            "PODCASTS",
            "RINGTONES"
        };
        String dirs[] = {
            Environment.DIRECTORY_ALARMS,
            Environment.DIRECTORY_DCIM,
            Environment.DIRECTORY_DOWNLOADS,
            Environment.DIRECTORY_MOVIES,
            Environment.DIRECTORY_MUSIC,
            Environment.DIRECTORY_NOTIFICATIONS,
            Environment.DIRECTORY_PICTURES,
            Environment.DIRECTORY_PODCASTS,
            Environment.DIRECTORY_RINGTONES
        };
        for (int i = 0; i < names.length; ++i) {
            File dir = Environment.getExternalStoragePublicDirectory(dirs[i]);
            if (null != dir) {
                nativeSetDirectory(names[i], dir.getPath());
            }
        }
    }

    public static void setCacheDirectory(String path) {
        nativeSetDirectory("CACHEDIR", path);
    }

    public static void setExternalCacheDirectory(String path) {
        nativeSetDirectory("EXTERNAL_CACHEDIR", path);
    }

    private static native void nativeSetDirectory(String virtualRoot, String path);
}
