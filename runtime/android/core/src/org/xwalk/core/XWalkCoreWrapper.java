// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.Signature;
import android.util.Log;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.HashMap;

import junit.framework.Assert;

/**
 * The appropriate invocation order is:
 * handlePreInit() - attachXWalkCore() - dockXWalkCore() - handlePostInit() - over
 */
class XWalkCoreWrapper {
    private static final String XWALK_APK_PACKAGE = "org.xwalk.core";
    private static final String WRAPPER_PACKAGE = "org.xwalk.core";
    private static final String BRIDGE_PACKAGE = "org.xwalk.core.internal";
    private static final String TAG = "XWalkLib";

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

    private Context mWrapperContext;
    private Context mBridgeContext;
    private ClassLoader mBridgeLoader;

    public static XWalkCoreWrapper getInstance() {
        return sInstance;
    }

    public static int getCoreStatus() {
        if (sInstance != null) return XWalkLibraryInterface.STATUS_MATCH;
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
        sProvisionalInstance = new XWalkCoreWrapper(context, -1);
        if (!sProvisionalInstance.findEmbeddedCore()) {
            int status = sProvisionalInstance.mCoreStatus;

            if (!sProvisionalInstance.findSharedCore()) {
                if (sProvisionalInstance.mCoreStatus != XWalkLibraryInterface.STATUS_NOT_FOUND) {
                    status = sProvisionalInstance.mCoreStatus;
                }
                Log.d(TAG, "core status: " + status);
                return status;
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
            Assert.fail("Please have your activity extend XWalkActivity for shared mode");
        }

        sInstance = provisionalInstance;
        sInstance.initCoreBridge();
    }

    private XWalkCoreWrapper(Context context, int minApiVersion) {
        mApiVersion = XWalkAppVersion.API_VERSION;
        mMinApiVersion = (minApiVersion > 0 && minApiVersion <= mApiVersion) ?
                minApiVersion : mApiVersion;
        mCoreStatus = XWalkLibraryInterface.STATUS_NOT_FOUND;
        mWrapperContext = context;
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

    private boolean findSharedCore() {
        if (!checkCorePackage()) return false;

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

    private boolean checkCoreVersion() {
        try {
            Class<?> clazz = getBridgeClass("XWalkCoreVersion");
            int libVersion = (int) new ReflectField(clazz, "API_VERSION").get();
            int minLibVersion = (int) new ReflectField(clazz, "MIN_API_VERSION").get();
            Log.d(TAG, "lib version, api:" + libVersion + ", min api:" + minLibVersion);
            Log.d(TAG, "app version, api:" + mApiVersion + ", min api:" + mMinApiVersion);

            if (mMinApiVersion > libVersion) {
                mCoreStatus = XWalkLibraryInterface.STATUS_OLDER_VERSION;
                return false;
            } else if (mApiVersion < minLibVersion) {
                mCoreStatus = XWalkLibraryInterface.STATUS_NEWER_VERSION;
                return false;
            }
        } catch (RuntimeException e) {
            Log.d(TAG, "Failed to check library version");
            mCoreStatus = XWalkLibraryInterface.STATUS_NOT_FOUND;
            return false;
        }

        return true;
    }

    private boolean checkCoreArchitecture() {
        try {
            Class<?> clazz = getBridgeClass("XWalkViewDelegate");
            new ReflectMethod(clazz, "loadXWalkLibrary", Context.class).invoke(mBridgeContext);
        } catch (RuntimeException e) {
            Log.d(TAG, "Failed to load native library");
            mCoreStatus = XWalkLibraryInterface.STATUS_ARCHITECTURE_MISMATCH;
            return false;
        }

        return true;
    }

    private boolean checkCorePackage() {
        if (mWrapperContext == null) {
            Log.d(TAG, "No application context");
            mCoreStatus = XWalkLibraryInterface.STATUS_NOT_FOUND;
            return false;
        }

        if (!XWalkAppVersion.VERIFY_XWALK_APK) {
            Log.d(TAG, "Not verifying the package integrity of Crosswalk runtime library");
        } else {
            try {
                PackageInfo packageInfo = mWrapperContext.getPackageManager().getPackageInfo(
                        XWALK_APK_PACKAGE, PackageManager.GET_SIGNATURES);
                if (!verifyPackageInfo(packageInfo,
                        XWalkAppVersion.XWALK_APK_HASH_ALGORITHM,
                        XWalkAppVersion.XWALK_APK_HASH_CODE)) {
                    mCoreStatus = XWalkLibraryInterface.STATUS_SIGNATURE_CHECK_ERROR;
                    return false;
                }
            } catch (NameNotFoundException e) {
                Log.d(TAG, "Crosswalk package not found");
                mCoreStatus = XWalkLibraryInterface.STATUS_NOT_FOUND;
                return false;
            }
        }

        try {
            mBridgeContext = mWrapperContext.createPackageContext(XWALK_APK_PACKAGE,
                    Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);
            Log.d(TAG, "Created bridge context");
        } catch (NameNotFoundException e) {
            Log.d(TAG, "Crosswalk package not found");
            mCoreStatus = XWalkLibraryInterface.STATUS_NOT_FOUND;
            return false;
        }

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
            Assert.fail("Invalid hash algorithm");
        }

        byte[] hashArray = hexStringToByteArray(hashCode);
        if (hashArray == null) {
            Assert.fail("Invalid hash code");
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
}
