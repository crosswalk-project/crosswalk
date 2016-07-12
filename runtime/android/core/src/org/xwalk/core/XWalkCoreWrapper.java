// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.Signature;
import android.content.SharedPreferences;
import android.os.Build;
import android.util.Log;
import dalvik.system.DexClassLoader;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.io.File;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.HashMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import junit.framework.Assert;

/**
 * The appropriate invocation order is:
 * handlePreInit() - attachXWalkCore() - dockXWalkCore() - handlePostInit() - over
 */
class XWalkCoreWrapper {
    private static final String WRAPPER_PACKAGE = "org.xwalk.core";
    private static final String BRIDGE_PACKAGE = "org.xwalk.core.internal";
    private static final String TAG = "XWalkLib";
    private static final String XWALK_CORE_CLASSES_DEX = "classes.dex";
    private static final String OPTIMIZED_DEX_DIR = "dex";
    private static final String META_XWALK_ENABLE_DOWNLOAD_MODE = "xwalk_enable_download_mode";
    private static final String META_XWALK_DOWNLOAD_MODE = "xwalk_download_mode";
    private static final String META_XWALK_DOWNLOAD_MODE_UPDATE = "xwalk_download_mode_update";
    private static final String PATTERN_BIT_MISMATCH =
            "dlopen failed: \".+\" is (32|64)-bit instead of (32|64)-bit";

    private static XWalkCoreWrapper sProvisionalInstance;
    private static XWalkCoreWrapper sInstance;

    private static LinkedList<String> sReservedActivities = new LinkedList<String>();
    private static HashMap<String, LinkedList<ReservedAction> > sReservedActions =
            new HashMap<String, LinkedList<ReservedAction> >();

    private static class ReservedAction {
        ReservedAction(Object object) {
            mObject = object;
        }

        ReservedAction(Class<?> clazz) {
            mClass = clazz;
        }

        ReservedAction(ReflectMethod method) {
            mMethod = method;
            if (method.getArguments() != null) {
                mArguments = Arrays.copyOf(method.getArguments(), method.getArguments().length);
            }
        }

        Object mObject;
        Class<?> mClass;
        ReflectMethod mMethod;
        Object[] mArguments;
    }

    private int mApiVersion;
    private int mMinApiVersion;
    private int mCoreStatus;
    private boolean mIsDownloadMode;

    private Context mWrapperContext;
    private Context mBridgeContext;
    private ClassLoader mBridgeLoader;

    public static XWalkCoreWrapper getInstance() {
        return sInstance;
    }

    public static int getCoreStatus() {
        if (sInstance != null) return sInstance.mCoreStatus;
        if (sProvisionalInstance == null) return XWalkLibraryInterface.STATUS_PENDING;
        return sProvisionalInstance.mCoreStatus;
    }

    /**
     * This method must be invoked on the UI thread.
     */
    public static void handlePreInit(String tag) {
        if (sInstance != null) return;

        Log.d(TAG, "Pre init xwalk core in " + tag);
        if (sReservedActions.containsKey(tag)) {
            sReservedActions.remove(tag);
        } else {
            sReservedActivities.add(tag);
        }

        sReservedActions.put(tag, new LinkedList<ReservedAction>());
    }

    public static void reserveReflectObject(Object object) {
        String tag = sReservedActivities.getLast();
        Log.d(TAG, "Reserve object " + object.getClass() + " to " + tag);
        sReservedActions.get(tag).add(new ReservedAction(object));
    }

    public static void reserveReflectClass(Class<?> clazz) {
        String tag = sReservedActivities.getLast();
        Log.d(TAG, "Reserve class " + clazz.toString() + " to " + tag);
        sReservedActions.get(tag).add(new ReservedAction(clazz));
    }

    public static void reserveReflectMethod(ReflectMethod method) {
        String tag = sReservedActivities.getLast();
        Log.d(TAG, "Reserve method " + method.toString() + " to " + tag);
        sReservedActions.get(tag).add(new ReservedAction(method));
    }

    public static void handleRuntimeError(RuntimeException e) {
        e.printStackTrace();
        sInstance.mCoreStatus = XWalkLibraryInterface.STATUS_OLDER_VERSION;
        final Activity activity = (Activity) sInstance.mWrapperContext;
        XWalkUpdater xwalkUpdater = new XWalkUpdater(
            new XWalkUpdater.XWalkUpdateListener() {
                @Override
                public void onXWalkUpdateCancelled() {
                    activity.finish();
                }
            },
            activity);
        xwalkUpdater.updateXWalkRuntime();
    }

    /**
     * This method must be invoked on the UI thread.
     */
    public static void handlePostInit(String tag) {
        if (!sReservedActions.containsKey(tag)) return;
        Log.d(TAG, "Post init xwalk core in " + tag);

        LinkedList<ReservedAction> reservedActions = sReservedActions.get(tag);
        for (ReservedAction action : reservedActions) {
            if (action.mObject != null) {
                Log.d(TAG, "Init reserved object: " + action.mObject.getClass());
                new ReflectMethod(action.mObject, "reflectionInit").invoke();
            } else if (action.mClass != null) {
                Log.d(TAG, "Init reserved class: " + action.mClass.toString());
                new ReflectMethod(action.mClass, "reflectionInit").invoke();
            } else {
                Log.d(TAG, "Call reserved method: " + action.mMethod.toString());
                Object[] args = action.mArguments;
                if (args != null) {
                    for (int i = 0; i < args.length; ++i) {
                        if (args[i] instanceof ReflectMethod) {
                            args[i] = ((ReflectMethod) args[i]).invokeWithArguments();
                        }
                    }
                }
                action.mMethod.invoke(args);
            }
        }

        sReservedActivities.remove(tag);
        sReservedActions.remove(tag);
    }

    public static int attachXWalkCore(Context context) {
        Assert.assertFalse(sReservedActivities.isEmpty());
        Assert.assertNull(sInstance);

        Log.d(TAG, "Attach xwalk core");
        sProvisionalInstance = new XWalkCoreWrapper(context, 1);
        if (sProvisionalInstance.findEmbeddedCore()) {
            return sProvisionalInstance.mCoreStatus;
        }

        if (sProvisionalInstance.mIsDownloadMode) {
            sProvisionalInstance.findDownloadedCore();
            return sProvisionalInstance.mCoreStatus;
        }

        if (!XWalkAppVersion.VERIFY_XWALK_APK) {
            Log.d(TAG, "Not verifying the package integrity of Crosswalk runtime library");
        }

        if (sProvisionalInstance.findSharedCore(XWalkLibraryInterface.XWALK_CORE_PACKAGE)) {
            return sProvisionalInstance.mCoreStatus;
        }

        String deviceAbi = getDeviceAbi();
        if (deviceAbi.equals("arm64-v8a")) {
            sProvisionalInstance.findSharedCore(XWalkLibraryInterface.XWALK_CORE64_PACKAGE);
        } else if (deviceAbi.equals("x86")) {
            sProvisionalInstance.findSharedCore(XWalkLibraryInterface.XWALK_CORE_IA_PACKAGE);
        } else if (deviceAbi.equals("x86_64")) {
            if (!sProvisionalInstance.findSharedCore(XWalkLibraryInterface.XWALK_CORE64_PACKAGE)) {
                sProvisionalInstance.findSharedCore(XWalkLibraryInterface.XWALK_CORE64_IA_PACKAGE);
            }
        }
        return sProvisionalInstance.mCoreStatus;
    }

    /**
     * This method must be invoked on the UI thread.
     */
    public static void dockXWalkCore() {
        Assert.assertNotNull(sProvisionalInstance);
        Assert.assertNull(sInstance);

        Log.d(TAG, "Dock xwalk core");
        sInstance = sProvisionalInstance;
        sProvisionalInstance = null;
        sInstance.initCoreBridge();
        sInstance.initXWalkView();
    }

    /**
     * This method must be invoked on the UI thread.
     */
    public static void initEmbeddedMode() {
        if (sInstance != null || !sReservedActivities.isEmpty()) return;

        Log.d(TAG, "Init embedded mode");
        XWalkCoreWrapper provisionalInstance = new XWalkCoreWrapper(null, -1);
        if (!provisionalInstance.findEmbeddedCore()) {
            throw new RuntimeException(
                    "Please have your activity extend XWalkActivity for shared mode");
        }

        sInstance = provisionalInstance;
        sInstance.initCoreBridge();
    }

    private XWalkCoreWrapper(Context context, int minApiVersion) {
        mApiVersion = XWalkAppVersion.API_VERSION;
        mMinApiVersion = (minApiVersion > 0 && minApiVersion <= mApiVersion) ?
                minApiVersion : mApiVersion;
        mCoreStatus = XWalkLibraryInterface.STATUS_PENDING;
        mWrapperContext = context;

        String enable = getApplicationMetaData(META_XWALK_DOWNLOAD_MODE);
        if (enable == null) {
            enable = getApplicationMetaData(META_XWALK_ENABLE_DOWNLOAD_MODE);
        }
        mIsDownloadMode = enable != null
                && (enable.equalsIgnoreCase("enable") || enable.equalsIgnoreCase("true"));
    }

    private void initCoreBridge() {
        Log.d(TAG, "Init core bridge");
        Class<?> clazz = getBridgeClass("XWalkCoreBridge");
        ReflectMethod method = new ReflectMethod(clazz, "init", Context.class, Object.class);
        method.invoke(mBridgeContext, this);
    }

    private void initXWalkView() {
        Log.d(TAG, "Init xwalk view");
        Class<?> clazz = getBridgeClass("XWalkViewDelegate");
        ReflectMethod method = new ReflectMethod(clazz, "init", Context.class, Context.class);
        method.invoke(mBridgeContext, mWrapperContext);
    }

    private boolean findEmbeddedCore() {
        mBridgeContext = null;

        mBridgeLoader = XWalkCoreWrapper.class.getClassLoader();
        if (!checkCoreVersion() || !checkCoreArchitecture()) {
            mBridgeLoader = null;
            return false;
        }

        Log.d(TAG, "Running in embedded mode");
        mCoreStatus = XWalkLibraryInterface.STATUS_MATCH;
        return true;
    }

    private boolean findSharedCore(String packageName) {
        if (!checkCorePackage(packageName)) return false;

        mBridgeLoader = mBridgeContext.getClassLoader();
        if (!checkCoreVersion() || !checkCoreArchitecture()) {
            mBridgeContext = null;
            mBridgeLoader = null;
            return false;
        }

        Log.d(TAG, "Running in shared mode");
        mCoreStatus = XWalkLibraryInterface.STATUS_MATCH;
        return true;
    }

    private boolean findDownloadedCore() {
        String libDir = mWrapperContext.getDir(XWalkLibraryInterface.XWALK_CORE_EXTRACTED_DIR,
                Context.MODE_PRIVATE).getAbsolutePath();
        String dexPath = libDir + File.separator + XWALK_CORE_CLASSES_DEX;
        String dexOutputPath = mWrapperContext.getDir(OPTIMIZED_DEX_DIR, Context.MODE_PRIVATE).
                getAbsolutePath();
        ClassLoader localClassLoader = ClassLoader.getSystemClassLoader();
        mBridgeLoader = new DexClassLoader(dexPath, dexOutputPath, libDir, localClassLoader);

        if (!checkCoreVersion() || !checkCoreArchitecture()) {
            mBridgeLoader = null;
            return false;
        }

        Log.d(TAG, "Running in downloaded mode");
        mCoreStatus = XWalkLibraryInterface.STATUS_MATCH;
        return true;
    }

    private boolean checkCoreVersion() {
        Log.d(TAG, "[App Version] build:" + XWalkAppVersion.XWALK_BUILD_VERSION
                + ", api:" + mApiVersion + ", min_api:" + mMinApiVersion);

        try {
            Class<?> clazz = getBridgeClass("XWalkCoreVersion");
            String buildVersion = "";
            try {
                buildVersion = (String) new ReflectField(clazz, "XWALK_BUILD_VERSION").get();
            } catch (RuntimeException e) {
            }
            int libVersion = (int) new ReflectField(clazz, "API_VERSION").get();
            int minLibVersion = (int) new ReflectField(clazz, "MIN_API_VERSION").get();
            Log.d(TAG, "[Lib Version] build:" + buildVersion
                    + ", api:" + libVersion + ", min_api:" + minLibVersion);

            if (mIsDownloadMode && !buildVersion.isEmpty()) {
                String update = getApplicationMetaData(META_XWALK_DOWNLOAD_MODE_UPDATE);
                if (update != null
                        && (update.equalsIgnoreCase("enable") || update.equalsIgnoreCase("true"))
                        && !buildVersion.equals(XWalkAppVersion.XWALK_BUILD_VERSION)) {
                    mCoreStatus = XWalkLibraryInterface.STATUS_RUNTIME_MISMATCH;
                    return false;
                }
            }

            if (mMinApiVersion > libVersion) {
                mCoreStatus = XWalkLibraryInterface.STATUS_OLDER_VERSION;
                return false;
            } else if (mApiVersion < minLibVersion) {
                mCoreStatus = XWalkLibraryInterface.STATUS_NEWER_VERSION;
                return false;
            }
        } catch (RuntimeException e) {
            Log.d(TAG, "XWalk core not found");
            mCoreStatus = XWalkLibraryInterface.STATUS_NOT_FOUND;
            return false;
        }

        Log.d(TAG, "XWalk core version matched");
        return true;
    }

    private boolean checkCoreArchitecture() {
        try {
            Class<?> clazz = getBridgeClass("XWalkViewDelegate");
            ReflectMethod method = new ReflectMethod(clazz, "loadXWalkLibrary",
                    Context.class, String.class);

            boolean architectureMatched = false;
            String libDir = null;
            if (mBridgeContext != null) {
                // Only load the native library from /data/data if in shared mode and the Android
                // version is lower than 4.2. Android enables a system path /data/app-lib to store
                // native libraries starting from 4.2 and load them automatically.
                if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN_MR1) {
                    libDir = "/data/data/" + mBridgeContext.getPackageName() + "/lib";
                }
                architectureMatched = (boolean) method.invoke(mBridgeContext, libDir);
            } else {
                try {
                    architectureMatched = (boolean) method.invoke(mBridgeContext, libDir);
                } catch (RuntimeException ex) {
                    Log.d(TAG, ex.getLocalizedMessage());
                }

                if (!architectureMatched && mWrapperContext != null) {
                    libDir = mWrapperContext.getDir(
                            XWalkLibraryInterface.PRIVATE_DATA_DIRECTORY_SUFFIX,
                            Context.MODE_PRIVATE).toString();
                    architectureMatched = (boolean) method.invoke(mBridgeContext, libDir);
                }
            }

            if (!architectureMatched) {
                Log.d(TAG, "Mismatch of CPU architecture");
                mCoreStatus = XWalkLibraryInterface.STATUS_ARCHITECTURE_MISMATCH;
                return false;
            }
        } catch (RuntimeException e) {
            Log.d(TAG, e.getLocalizedMessage());
            if (e.getCause() instanceof UnsatisfiedLinkError) {
                mCoreStatus = XWalkLibraryInterface.STATUS_ARCHITECTURE_MISMATCH;
                return false;
            }
            mCoreStatus = XWalkLibraryInterface.STATUS_INCOMPLETE_LIBRARY;
            return false;
        }

        Log.d(TAG, "XWalk core architecture matched");
        return true;
    }

    private boolean checkCorePackage(String packageName) {
        if (XWalkAppVersion.VERIFY_XWALK_APK) {
            try {
                PackageInfo packageInfo = mWrapperContext.getPackageManager().getPackageInfo(
                        packageName, PackageManager.GET_SIGNATURES);
                if (!verifyPackageInfo(packageInfo,
                        XWalkAppVersion.XWALK_APK_HASH_ALGORITHM,
                        XWalkAppVersion.XWALK_APK_HASH_CODE)) {
                    Log.d(TAG, packageName + " signature verification failed");
                    mCoreStatus = XWalkLibraryInterface.STATUS_SIGNATURE_CHECK_ERROR;
                    return false;
                }
            } catch (NameNotFoundException e) {
                Log.d(TAG, packageName + " not found");
                return false;
            }
        }

        try {
            mBridgeContext = mWrapperContext.createPackageContext(packageName,
                    Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);
        } catch (NameNotFoundException e) {
            Log.d(TAG, packageName + " not found");
            return false;
        }

        Log.d(TAG, "Created package context for " + packageName);
        return true;
    }

    private boolean verifyPackageInfo(PackageInfo packageInfo, String hashAlgorithm,
            String hashCode) {
        if (packageInfo.signatures == null) {
            Log.e(TAG, "No signature in package info");
            return false;
        }

        MessageDigest md = null;
        try {
            md = MessageDigest.getInstance(hashAlgorithm);
        } catch (NoSuchAlgorithmException | NullPointerException e) {
            throw new IllegalArgumentException("Invalid hash algorithm");
        }

        byte[] hashArray = hexStringToByteArray(hashCode);
        if (hashArray == null) {
            throw new IllegalArgumentException("Invalid hash code");
        }

        for (int i = 0; i < packageInfo.signatures.length; ++i) {
            Log.d(TAG, "Checking signature " + i);
            byte[] binaryCert = packageInfo.signatures[i].toByteArray();
            byte[] digest = md.digest(binaryCert);
            if (!MessageDigest.isEqual(digest, hashArray)) {
                Log.e(TAG, "Hash code does not match");
                continue;
            }

            Log.d(TAG, "Signature passed verification");
            return true;
        }

        return false;
    }

    private byte[] hexStringToByteArray(String str) {
        if (str == null || str.isEmpty() || str.length()%2 != 0) return null;

        byte[] result = new byte[str.length() / 2];
        for (int i = 0; i < str.length(); i += 2) {
            int digit = Character.digit(str.charAt(i), 16);
            digit <<= 4;
            digit += Character.digit(str.charAt(i+1), 16);
            result[i/2] = (byte) digit;
        }
        return result;
    }

    public boolean isSharedMode() {
        return mBridgeContext != null;
    }

    public Object getBridgeObject(Object object) {
        try {
            return new ReflectMethod(object, "getBridge").invoke();
        } catch (RuntimeException e) {
        }
        return null;
    }

    public Object getWrapperObject(Object object) {
        try {
            return new ReflectMethod(object, "getWrapper").invoke();
        } catch (RuntimeException e) {
        }
        return null;
    }

    public Class<?> getBridgeClass(String name) {
        try {
            return mBridgeLoader.loadClass(BRIDGE_PACKAGE + "." + name);
        } catch (ClassNotFoundException e) {
        }
        return null;
    }

    private String getApplicationMetaData(String name) {
        try {
            PackageManager packageManager = mWrapperContext.getPackageManager();
            ApplicationInfo appInfo = packageManager.getApplicationInfo(
                    mWrapperContext.getPackageName(), PackageManager.GET_META_DATA);
            return appInfo.metaData.getString(name);
        } catch (NameNotFoundException | NullPointerException e) {
        }
        return null;
    }

    public static String getRuntimeAbi() {
        String runtimeAbi = System.getProperty("os.arch").toLowerCase();
        switch (runtimeAbi) {
            case "x86":
            case "i686":
            case "i386":
            case "ia32":
                return "x86";
            case "x64":
            case "x86_64":
                return "x86_64";
            case "armv7l":
            case "armeabi":
            case "armeabi-v7a":
                return "armeabi-v7a";
            case "aarch64":
            case "armv8":
            case "arm64":
                return "arm64-v8a";
            default:
                throw new RuntimeException("Unexpected runtime ABI: " + runtimeAbi);
        }
    }

    public static String getDeviceAbi() {
        String deviceAbi = "";
        try {
            deviceAbi = Build.SUPPORTED_ABIS[0].toLowerCase();
        } catch (NoSuchFieldError e) {
            try {
                Process process = Runtime.getRuntime().exec("getprop ro.product.cpu.abi");
                InputStreamReader ir = new InputStreamReader(process.getInputStream());
                BufferedReader input = new BufferedReader(ir);
                deviceAbi = input.readLine().toLowerCase();
                input.close();
                ir.close();
            } catch (IOException ex) {
                // CPU_ABI is deprecated in API level 21 and maybe incorrect on Houdini
                deviceAbi = Build.CPU_ABI.toLowerCase();
            }
        }

        switch (deviceAbi) {
            case "x86":
                return "x86";
            case "x86_64":
                return "x86_64";
            case "armeabi":
            case "armeabi-v7a":
                return "armeabi-v7a";
            case "arm64-v8a":
                return "arm64-v8a";
            default:
                throw new RuntimeException("Unexpected device ABI: " + deviceAbi);
        }
    }
}
