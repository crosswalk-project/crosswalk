// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.IOException;
import java.lang.StringBuilder;
import java.util.HashSet;
import java.util.Set;

import android.content.Context;
import android.content.res.AssetManager;
import android.content.res.Resources.NotFoundException;
import android.os.Build;
import android.util.Log;

import org.chromium.base.ApplicationStatusManager;
import org.chromium.base.CommandLine;
import org.chromium.base.JNINamespace;
import org.chromium.base.PathUtils;
import org.chromium.base.ThreadUtils;
import org.chromium.base.library_loader.LibraryLoader;
import org.chromium.base.library_loader.ProcessInitException;
import org.chromium.content.browser.BrowserStartupController;
import org.chromium.content.browser.DeviceUtils;
import org.chromium.content.browser.ResourceExtractor;
import org.chromium.content.browser.ResourceExtractor.ResourceIntercepter;
import org.chromium.net.NetworkChangeNotifier;

@JNINamespace("xwalk")
class XWalkViewDelegate {
    private static boolean sInitialized = false;
    private static boolean sRunningOnIA = true;
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

    private static final String COMMAND_LINE_FILE = "xwalk-command-line";

    private static String[] readCommandLine(Context context) {
        InputStreamReader reader = null;

        try {
            InputStream input =
                    context.getAssets().open(COMMAND_LINE_FILE, AssetManager.ACCESS_BUFFER);
            int length;
            int size = 1024;
            char[] buffer = new char[size];
            StringBuilder builder = new StringBuilder();

            reader = new InputStreamReader(input, "UTF-8");
            while ((length = reader.read(buffer, 0, size)) != -1) {
                builder.append(buffer, 0, length);
            }

            return CommandLine.tokenizeQuotedAruments(
                    builder.toString().toCharArray());
        } catch (IOException e) {
            return null;
        } finally {
            try {
                if (reader != null) reader.close();
            } catch (IOException e) {
                Log.e(TAG, "Unable to close file reader.", e);
            }
        }
    }

    public static void init(XWalkView xwalkView) throws UnsatisfiedLinkError {
        if (sInitialized) {
            return;
        }

        // Initialize the ActivityStatus. This is needed and used by many internal
        // features such as location provider to listen to activity status.
        ApplicationStatusManager.init(xwalkView.getActivity().getApplication());

        // Auto detect network connectivity state.
        // setAutoDetectConnectivityState() need to be called before activity started.
        NetworkChangeNotifier.init(xwalkView.getActivity());
        NetworkChangeNotifier.setAutoDetectConnectivityState(true);

        // We will miss activity onCreate() status in ApplicationStatusManager,
        // informActivityStarted() will simulate these callbacks.
        ApplicationStatusManager.informActivityStarted(xwalkView.getActivity());

        final Context context = xwalkView.getViewContext();

        // Last place to initialize CommandLine object. If you haven't initialize
        // the CommandLine object before XWalkViewContent is created, here will create
        // the object to guarantee the CommandLine object is not null and the
        // consequent prodedure does not crash.
        if (!CommandLine.isInitialized()) {
            CommandLine.init(readCommandLine(context.getApplicationContext()));
        }

        // If context's applicationContext is not the same package with itself,
        // It's a cross package invoking, load core library from library apk.
        // Only load the native library from /data/data if the Android version is
        // lower than 4.2. Android enables a system path /data/app-lib to store native
        // libraries starting from 4.2 and load them automatically.
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN_MR1 &&
                !context.getApplicationContext().getPackageName().equals(context.getPackageName())) {
            for (String library : MANDATORY_LIBRARIES) {
                System.load("/data/data/" + context.getPackageName() + "/lib/" + library);
            }
        }
        loadLibrary(context);
        DeviceUtils.addDeviceSpecificUserAgentSwitch(context);

        if (sRunningOnIA && !nativeIsLibraryBuiltForIA()) {
            throw new UnsatisfiedLinkError();
        }

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
            LibraryLoader.loadNow(context, true);
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

    public static boolean isRunningOnIA() {
        return sRunningOnIA;
    }

    private static native boolean nativeIsLibraryBuiltForIA();

    static {
        sRunningOnIA = Build.CPU_ABI.equalsIgnoreCase("x86");
    }
}
