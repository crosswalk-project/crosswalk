// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.Context;
import android.os.Build;

import org.chromium.base.PathUtils;
import org.chromium.base.ThreadUtils;
import org.chromium.content.app.LibraryLoader;
import org.chromium.content.browser.AndroidBrowserProcess;
import org.chromium.content.browser.DeviceUtils;
import org.chromium.content.browser.ResourceExtractor;
import org.chromium.content.common.CommandLine;
import org.chromium.content.common.ProcessInitException;

class XWalkViewDelegate {
    private static boolean sInitialized = false;
    private static final String PRIVATE_DATA_DIRECTORY_SUFFIX = "xwalkcore";
    private static final String[] MANDATORY_PAKS = {
            "xwalk.pak", "en-US.pak" };
    private static final String[] MANDATORY_LIBRARIES = {
            "libxwalkcore.so"
    };

    public static void init(Context context) {
        if (sInitialized) {
            return;
        }
        // Last place to initialize CommandLine object. If you haven't initialize
        // the CommandLine object before XWalkViewContent is created, here will create
        // the object to guarantee the CommandLine object is not null and the
        // consequent prodedure does not crash.
        if (!CommandLine.isInitialized())
            CommandLine.init(null);

        ResourceExtractor.setMandatoryPaksToExtract(MANDATORY_PAKS);
        ResourceExtractor.setExtractImplicitLocaleForTesting(false);
        // If context's applicationContext is not the same package with itself,
        // It's a cross package invoking, load core library from library apk.
        // Only load the native library from /data/data if the Android version is
        // lower than 4.2. Android enables a system path /data/app-lib to store native
        // libraries starting from 4.2 and load them automatically.
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN_MR1 &&
                !context.getApplicationContext().getPackageName().equals(context.getPackageName())) {
            try {
                for (String library : MANDATORY_LIBRARIES) {
                    System.load("/data/data/" + context.getPackageName() + "/lib/" + library);
                }
            } catch (UnsatisfiedLinkError e) {
                throw new RuntimeException("Cannot initialize Crosswalk Core", e);
            }
        }
        loadLibrary();
        DeviceUtils.addDeviceSpecificUserAgentSwitch(context);
        startBrowserProcess(context);
        sInitialized = true;
    }

    private static void loadLibrary() {
        PathUtils.setPrivateDataDirectorySuffix(PRIVATE_DATA_DIRECTORY_SUFFIX);
        try {
            LibraryLoader.loadNow();
        } catch (ProcessInitException e) {
            throw new RuntimeException("Cannot load Crosswalk Core", e);
        }
    }

    private static void startBrowserProcess(final Context context) {
        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                try {
                    LibraryLoader.ensureInitialized();
                    AndroidBrowserProcess.init(context,
                            AndroidBrowserProcess.MAX_RENDERERS_SINGLE_PROCESS);
                } catch (ProcessInitException e) {
                    throw new RuntimeException("Cannot initialize Crosswalk Core", e);
                }
            }
        });
    }
}
