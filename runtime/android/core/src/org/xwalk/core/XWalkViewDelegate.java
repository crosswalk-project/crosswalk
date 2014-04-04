// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import java.io.InputStream;
import java.util.HashSet;
import java.util.Set;

import android.content.Context;
import android.content.res.Resources.NotFoundException;
import android.os.Build;
import android.util.Log;

import org.chromium.base.ApplicationStatus;
import org.chromium.base.ApplicationStatusManager;
import org.chromium.base.CommandLine;
import org.chromium.base.PathUtils;
import org.chromium.base.ThreadUtils;
import org.chromium.base.library_loader.LibraryLoader;
import org.chromium.base.library_loader.ProcessInitException;
import org.chromium.content.browser.BrowserStartupController;
import org.chromium.content.browser.DeviceUtils;
import org.chromium.content.browser.ResourceExtractor;
import org.chromium.content.browser.ResourceExtractor.ResourceIntercepter;

class XWalkViewDelegate {
    private static boolean sInitialized = false;
    private static final String PRIVATE_DATA_DIRECTORY_SUFFIX = "xwalkcore";
    private static final String[] MANDATORY_PAKS = {
            "xwalk.pak",
            "en-US.pak",
            "icudtl.dat"
    };
    private static final String[] MANDATORY_LIBRARIES = {
            "libxwalkcore.so"
    };
    private static final String TAG = "XWalkViewDelegate";
    private static final String XWALK_RESOURCES_LIST_RES_NAME = "xwalk_resources_list";

    public static void init(XWalkView xwalkView) {
        if (sInitialized) {
            return;
        }

        // Initialize the ActivityStatus. This is needed and used by many internal
        // features such as location provider to listen to activity status.
        ApplicationStatusManager.init(xwalkView.getActivity().getApplication());

        final Context context = xwalkView.getViewContext();

        // Last place to initialize CommandLine object. If you haven't initialize
        // the CommandLine object before XWalkViewContent is created, here will create
        // the object to guarantee the CommandLine object is not null and the
        // consequent prodedure does not crash.
        if (!CommandLine.isInitialized()) {
            CommandLine.init(null);
        }

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
        loadLibrary(context);
        DeviceUtils.addDeviceSpecificUserAgentSwitch(context);

        ResourceExtractor.setMandatoryPaksToExtract(MANDATORY_PAKS);
        final int resourcesListResId = context.getResources().getIdentifier(
                XWALK_RESOURCES_LIST_RES_NAME, "array", context.getPackageName());
        if (resourcesListResId != 0) {
            ResourceExtractor.setResourceIntercepter(new ResourceIntercepter() {

                @Override
                public Set<String> getInterceptableResourceList() {
                    try {
                        Set<String> resourcesList = new HashSet<String>();
                        String[] resources = context.getResources().getStringArray(resourcesListResId);
                        for (String resource : resources) {
                            resourcesList.add(resource);
                        }
                        return resourcesList;
                    } catch (NotFoundException e) {
                        Log.w(TAG, "R.array." + XWALK_RESOURCES_LIST_RES_NAME + " can't be found.");
                    }
                    return null;
                }

                @Override
                public InputStream interceptLoadingForResource(String resource) {
                    String resourceName = resource.split("\\.")[0];
                    int resId = context.getResources().getIdentifier(
                            resourceName, "raw", context.getPackageName());
                    try {
                        if (resId != 0) return context.getResources().openRawResource(resId);
                    } catch (NotFoundException e) {
                        Log.w(TAG, "R.raw." + resourceName + " can't be found.");
                    }
                    return null;
                }
            });
        }
        ResourceExtractor.setExtractImplicitLocaleForTesting(false);
        // Use MixedContext to initialize the ResourceExtractor, as the pak file
        // is in the library apk if in shared apk mode.
        ResourceExtractor.get(context);

        startBrowserProcess(context);
        sInitialized = true;
    }

    private static void loadLibrary(Context context) {
        PathUtils.setPrivateDataDirectorySuffix(PRIVATE_DATA_DIRECTORY_SUFFIX);
        try {
            LibraryLoader.loadNow(context);
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
                } catch (ProcessInitException e) {
                    throw new RuntimeException("Cannot initialize Crosswalk Core", e);
                }
                try {
                    BrowserStartupController.get(context).startBrowserProcessesSync(
                        BrowserStartupController.MAX_RENDERERS_SINGLE_PROCESS);
                } catch (ProcessInitException e) {
                    throw new RuntimeException("Cannot initialize Crosswalk Core", e);
                }
            }
        });
    }
}
