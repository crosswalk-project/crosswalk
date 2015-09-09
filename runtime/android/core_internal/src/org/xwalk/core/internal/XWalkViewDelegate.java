// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.IOException;
import java.lang.StringBuilder;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;

import android.content.Context;
import android.content.res.AssetManager;
import android.content.res.Resources.NotFoundException;
import android.os.Build;
import android.util.Log;

import junit.framework.Assert;

import org.chromium.base.CommandLine;
import org.chromium.base.JNINamespace;
import org.chromium.base.PathUtils;
import org.chromium.base.ResourceExtractor;
import org.chromium.base.ResourceExtractor.ResourceEntry;
import org.chromium.base.ResourceExtractor.ResourceInterceptor;
import org.chromium.base.ThreadUtils;
import org.chromium.base.library_loader.LibraryLoader;
import org.chromium.base.library_loader.LibraryProcessType;
import org.chromium.base.library_loader.ProcessInitException;
import org.chromium.content.browser.BrowserStartupController;
import org.chromium.content.browser.DeviceUtils;

@JNINamespace("xwalk")
class XWalkViewDelegate {
    private static boolean sInitialized = false;
    private static boolean sLibraryLoaded = false;
    private static boolean sRunningOnIA = true;
    private static final String PRIVATE_DATA_DIRECTORY_SUFFIX = "xwalkcore";

    // TODO(rakuco,lincsoon): This list is also in generate_xwalk_core_library.py.
    // We should remove it from one of the places to avoid duplication.
    private static final String[] MANDATORY_PAKS = {
            "xwalk.pak",
            "icudtl.dat",
            // Please refer to XWALK-3516, disable v8 use external startup data,
            // reopen it if needed later.
            // "natives_blob.bin",
            // "snapshot_blob.bin"
    };

    private static final String[] MANDATORY_LIBRARIES = {
        "xwalkcore"
    };
    private static final String TAG = "XWalkViewDelegate";
    private static final String XWALK_RESOURCES_LIST_RES_NAME = "xwalk_resources_list";
    private static final String XWALK_PAK_NAME = "xwalk.pak";

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

    public static void init(Context bridgeContext, Context context) {
        loadXWalkLibrary(bridgeContext);

        try {
            if (bridgeContext == null) {
                init(context);
            } else {
                init(new MixedContext(bridgeContext, context));
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public static void loadXWalkLibrary(Context context) throws UnsatisfiedLinkError {
        if (sLibraryLoaded) return;

        // If context is null, it's running in embedded mode, otherwise in shared mode.
        // Only load the native library from /data/data if in shared mode and the Android version
        // is lower than 4.2. Android enables a system path /data/app-lib to store native
        // libraries starting from 4.2 and load them automatically.
        try {
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN_MR1 && context != null) {
                String path = "/data/data/" + context.getPackageName() + "/lib/";
                for (String library : MANDATORY_LIBRARIES) {
                    System.load(path + "lib" + library + ".so");
                }
            } else {
                for (String library : MANDATORY_LIBRARIES) {
                    System.loadLibrary(library);
                }
            }
        } catch (UnsatisfiedLinkError e) {
            // The libxwalkcore.so doesn't exist in default directory if it's decompressed. In this
            // case, it will be loaded through org.xwalk.core.XWalkLibraryDecompressor.
        }

        // Load libraries what is wrote in NativeLibraries.java at compile time. It may duplicate
        // with System.loadLibrary("xwalkcore") above, but same library won't be loaded repeatedly.
        try {
            LibraryLoader libraryLoader = LibraryLoader.get(LibraryProcessType.PROCESS_BROWSER);
            libraryLoader.loadNow(context);
        } catch (ProcessInitException e) {
        }

        if (sRunningOnIA != nativeIsLibraryBuiltForIA()) {
            throw new UnsatisfiedLinkError();
        }
        sLibraryLoaded = true;
    }

    private static void init(final Context context) throws IOException {
        if (sInitialized) return;

        PathUtils.setPrivateDataDirectorySuffix(PRIVATE_DATA_DIRECTORY_SUFFIX, context);

        // Initialize chromium resources. Assign them the correct ids in xwalk core.
        XWalkInternalResources.resetIds(context);

        // Last place to initialize CommandLine object. If you haven't initialize
        // the CommandLine object before XWalkViewContent is created, here will create
        // the object to guarantee the CommandLine object is not null and the
        // consequent prodedure does not crash.
        if (!CommandLine.isInitialized()) {
            CommandLine.init(readCommandLine(context.getApplicationContext()));
        }

        setupResourceInterceptor(context);

        // Use MixedContext to initialize the ResourceExtractor, as the pak file
        // is in the library apk if in shared apk mode.
        ResourceExtractor.get(context);

        startBrowserProcess(context);
        sInitialized = true;
    }

    private static void startBrowserProcess(final Context context) {
        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                try {
                    LibraryLoader.get(LibraryProcessType.PROCESS_BROWSER).ensureInitialized(context);
                } catch (ProcessInitException e) {
                    throw new RuntimeException("Cannot initialize Crosswalk Core", e);
                }
                DeviceUtils.addDeviceSpecificUserAgentSwitch(context);
                CommandLine.getInstance().appendSwitchWithValue(
                        XWalkSwitches.PROFILE_NAME,
                        XWalkPreferencesInternal.getStringValue(XWalkPreferencesInternal.PROFILE_NAME));

                if (XWalkPreferencesInternal.getValue(XWalkPreferencesInternal.ANIMATABLE_XWALK_VIEW) &&
                        !CommandLine.getInstance().hasSwitch(XWalkSwitches.DISABLE_GPU_RASTERIZATION)) {
                    CommandLine.getInstance().appendSwitch(XWalkSwitches.DISABLE_GPU_RASTERIZATION);
                }

                try {
                    BrowserStartupController.get(context, LibraryProcessType.PROCESS_BROWSER).
                        startBrowserProcessesSync(true);
                } catch (ProcessInitException e) {
                    throw new RuntimeException("Cannot initialize Crosswalk Core", e);
                }
            }
        });
    }

    /**
     * Plugs an instance of ResourceExtractor.ResourceIntercepter() into ResourceExtractor.
     *
     * It is responsible for loading resources from the right locations depending on whether
     * Crosswalk is being used in shared or embedded mode.
     */
    private static void setupResourceInterceptor(final Context context) throws IOException {
        final boolean isSharedMode =
                !context.getPackageName().equals(context.getApplicationContext().getPackageName());

        // The test APKs (XWalkCoreShell, XWalkCoreInternalShell etc) are different from normal
        // Crosswalk apps: even though they use Crosswalk in embedded mode, the resources are stored
        // in assets/ with the rest of the app's assets.
        // XWalkRuntimeClientShell is the only exception, as it uses Crosswalk in shared mode.
        final boolean isTestApk =
                !isSharedMode && Arrays.asList(context.getAssets().list("")).contains(XWALK_PAK_NAME);

        HashMap<String, ResourceEntry> resourceList = new HashMap<String, ResourceEntry>();
        if (isSharedMode || isTestApk) {
            for (String resource : MANDATORY_PAKS) {
                resourceList.put(resource, new ResourceEntry(0, "", resource));
            }
        } else {
            int resourceListId = getResourceId(context, XWALK_RESOURCES_LIST_RES_NAME, "array");
            try {
                final String[] crosswalkResources = context.getResources().getStringArray(resourceListId);
                for (String resource : crosswalkResources) {
                    resourceList.put(resource, new ResourceEntry(0, "", resource));
                }
            } catch (NotFoundException e) {
                Assert.fail("R.array." + XWALK_RESOURCES_LIST_RES_NAME + " can't be found.");
            }
        }
        ResourceExtractor.setResourcesToExtract(
                resourceList.values().toArray(new ResourceEntry[resourceList.size()]));

        // For shouldInterceptLoadRequest(), which needs a final value.
        final HashSet<String> interceptableResources = new HashSet<String>(resourceList.keySet());

        // For shared mode, assets are in library package.
        // For embedded mode, assets are in res/raw.
        ResourceExtractor.setResourceInterceptor(new ResourceInterceptor() {
            @Override
            public boolean shouldInterceptLoadRequest(String resource) {
                return interceptableResources.contains(resource);
            }

            @Override
            public InputStream openRawResource(String resource) {
                if (isSharedMode || isTestApk) {
                    try {
                        return context.getAssets().open(resource);
                    } catch (IOException e) {
                        Assert.fail(resource + " can't be found in assets.");
                    }
                } else {
                    String resourceName = resource.split("\\.")[0];
                    int resourceId = getResourceId(context, resourceName, "raw");
                    try {
                        return context.getResources().openRawResource(resourceId);
                    } catch (NotFoundException e) {
                        Assert.fail("R.raw." + resourceName + " can't be found.");
                    }
                }
                return null;
            }
        });
    }

    /**
     * Returns a resource identifier for a given resource name and type.
     *
     * Basically a wrapper around Resources.getIdentifier() that also works with applications that
     * change their package name at build time (see XWALK-3569).
     */
    private static int getResourceId(final Context context, final String resourceName, final String resourceType) {
        int resourceId = context.getResources().getIdentifier(
                resourceName, resourceType, context.getClass().getPackage().getName());
        if (resourceId == 0) {
            resourceId = context.getResources().getIdentifier(
                    resourceName, resourceType, context.getPackageName());
        }
        return resourceId;
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
