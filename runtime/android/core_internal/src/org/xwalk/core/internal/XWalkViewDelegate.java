// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import java.io.BufferedReader;
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
import org.chromium.base.ResourceExtractor;
import org.chromium.base.ResourceExtractor.ResourceIntercepter;
import org.chromium.base.ThreadUtils;
import org.chromium.base.library_loader.LibraryLoader;
import org.chromium.base.library_loader.ProcessInitException;
import org.chromium.content.browser.BrowserStartupController;
import org.chromium.content.browser.DeviceUtils;
import org.chromium.net.NetworkChangeNotifier;

@JNINamespace("xwalk")
class XWalkViewDelegate {
    private static boolean sInitialized = false;
    private static boolean sLibraryLoaded = false;
    private static boolean sRunningOnIA = true;
    private static final String PRIVATE_DATA_DIRECTORY_SUFFIX = "xwalkcore";
    private static final String[] MANDATORY_PAKS = {
            "xwalk.pak",
            "en-US.pak",
            "icudtl.dat",
            "natives_blob.bin",
            "snapshot_blob.bin"
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

    public static void loadXWalkLibrary(Context context) throws UnsatisfiedLinkError {
        if (sLibraryLoaded) return;

        // If context is null, it's called from wrapper's ReflectionHelper to try
        // loading native library within the package. No need to try load from library
        // package in this case.
        // If context's applicationContext is not the same package with itself,
        // It's a cross package invoking, load core library from library apk.
        // Only load the native library from /data/data if the Android version is
        // lower than 4.2. Android enables a system path /data/app-lib to store native
        // libraries starting from 4.2 and load them automatically.
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN_MR1 && context != null &&
                !context.getApplicationContext().getPackageName().equals(context.getPackageName())) {
            for (String library : MANDATORY_LIBRARIES) {
                System.load("/data/data/" + context.getPackageName() + "/lib/" + library);
            }
        }
        loadLibrary(context);

        if (sRunningOnIA && !nativeIsLibraryBuiltForIA()) {
            throw new UnsatisfiedLinkError();
        }
        sLibraryLoaded = true;
    }

    public static void init(XWalkViewInternal xwalkView) throws UnsatisfiedLinkError {
        if (sInitialized) {
            return;
        }

        loadXWalkLibrary(xwalkView.getContext());

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

        ResourceExtractor.setMandatoryPaksToExtract(MANDATORY_PAKS);
        final int resourcesListResId = context.getResources().getIdentifier(
                XWALK_RESOURCES_LIST_RES_NAME, "array", context.getPackageName());
        final AssetManager assets = context.getAssets();
        if (!context.getPackageName().equals(context.getApplicationContext().getPackageName()) ||
                resourcesListResId != 0) {
            // For shared mode, assets are in library package.
            // For embedding API usage, assets are in res/raw.
            ResourceExtractor.setResourceIntercepter(new ResourceIntercepter() {

                @Override
                public Set<String> getInterceptableResourceList() {
                    Set<String> resourcesList = new HashSet<String>();
                    if (!context.getPackageName().equals(
                            context.getApplicationContext().getPackageName())) {
                        try {
                            for (String resource : assets.list("")) {
                                resourcesList.add(resource);
                            }
                        } catch (IOException e){}
                    }
                    if (resourcesListResId != 0) {
                        try {
                            String[] resources = context.getResources().getStringArray(resourcesListResId);
                            for (String resource : resources) {
                                resourcesList.add(resource);
                            }
                        } catch (NotFoundException e) {
                            Log.w(TAG, "R.array." + XWALK_RESOURCES_LIST_RES_NAME + " can't be found.");
                        }
                    }
                    return resourcesList;
                }

                @Override
                public InputStream interceptLoadingForResource(String resource) {
                    if (!context.getPackageName().equals(
                            context.getApplicationContext().getPackageName())) {
                        try {
                            InputStream fromAsset = context.getAssets().open(resource);
                            if (fromAsset != null) return fromAsset;
                        } catch (IOException e) {
                            Log.w(TAG, resource + " can't be found in assets.");
                        }
                    }

                    if (resourcesListResId != 0) {
                        String resourceName = resource.split("\\.")[0];
                        int resId = context.getResources().getIdentifier(
                                resourceName, "raw", context.getPackageName());
                        try {
                            if (resId != 0) return context.getResources().openRawResource(resId);
                        } catch (NotFoundException e) {
                            Log.w(TAG, "R.raw." + resourceName + " can't be found.");
                        }
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
                DeviceUtils.addDeviceSpecificUserAgentSwitch(context);
                CommandLine.getInstance().appendSwitchWithValue(
                        XWalkSwitches.PROFILE_NAME,
                        XWalkPreferencesInternal.getStringValue(XWalkPreferencesInternal.PROFILE_NAME));
                try {
                    BrowserStartupController.get(context).startBrowserProcessesSync(
                        true);
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
        sRunningOnIA = Build.CPU_ABI.equalsIgnoreCase("x86") || Build.CPU_ABI.equalsIgnoreCase("x86_64");
        if (!sRunningOnIA) {
            // This is not the final decision yet.
            // With latest Houdini, an app with ARM binary will see system abi as if it's running on
            // arm device. Here needs some further check for real system abi.
            try {
                Process process = Runtime.getRuntime().exec("getprop ro.product.cpu.abi");
                InputStreamReader ir = new InputStreamReader(process.getInputStream());
                BufferedReader input = new BufferedReader(ir);
                String abi = input.readLine();
                sRunningOnIA = abi.contains("x86");
                input.close();
                ir.close();
            } catch (IOException e) {
                Log.w(TAG, Log.getStackTraceString(e));
            }
        }
    }
}
