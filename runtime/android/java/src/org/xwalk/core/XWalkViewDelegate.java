// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.Context;

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
