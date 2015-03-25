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

import junit.framework.Assert;

import org.xwalk.core.XWalkLibraryListener.LibraryStatus;

class XWalkCoreWrapper implements ReflectExceptionHandler {
    public static final String XWALK_APK_PACKAGE = "org.xwalk.core";
    public static final String WRAPPER_PACKAGE = "org.xwalk.core";
    public static final String BRIDGE_PACKAGE = "org.xwalk.core.internal";

    private static XWalkCoreWrapper sProvisionalInstance;
    private static XWalkCoreWrapper sInstance;
    private static XWalkLibraryListener sListener;

    private static final String TAG = "XWalkActivity";

    private int mSdkVersion;
    private int mMinSdkVersion;

    private Object mCore;
    private LibraryStatus mCoreStatus;
    private ClassLoader mBridgeLoader;
    private Context mBridgeContext;

    public static XWalkCoreWrapper getInstance() {
        return sInstance;
    }

    public static void initEmbeddedMode() {
        if (sInstance != null || sProvisionalInstance != null || sListener != null) return;

        sProvisionalInstance = new XWalkCoreWrapper(-1);
        if (!sProvisionalInstance.findEmbeddedCore()) {
            Assert.fail("Must extend XWalkActivity on shared mode");
        }

        init();
        Log.d(TAG, "Initialized embedded mode without XWalkActivity");
    }

    public static void check() {
        check(-1);
    }

    public static void check(int minSdkVersion) {
        Assert.assertNull(sInstance);
        sProvisionalInstance = new XWalkCoreWrapper(minSdkVersion);
        if (!sProvisionalInstance.findEmbeddedCore()) {
            sProvisionalInstance.findSharedCore();
        }
        if (sListener == null) return;

        LibraryStatus status = sProvisionalInstance.mCoreStatus;
        if (status == LibraryStatus.MATCHED) {
            sListener.onXWalkLibraryMatched();
            return;
        }

        Error error = (status == LibraryStatus.NOT_FOUND) ?
                new UnsatisfiedLinkError("XWalk Core Not Found") :
                new VerifyError("API Incompatible");
        sListener.onXWalkLibraryStartupError(status, error);
    }

    public static void init() {
        Assert.assertNull(sInstance);
        Assert.assertNotNull(sProvisionalInstance);
        sInstance = sProvisionalInstance;
        sProvisionalInstance = null;

        sInstance.initCore();
    }

    public static void reset(XWalkCoreWrapper coreWrapper, XWalkLibraryListener coreListener) {
        sProvisionalInstance = null;
        sInstance = coreWrapper;
        sListener = coreListener;

        if (sInstance != null) sInstance.resetCore();
    }

    public static boolean reserveReflectObject(Object object) {
        if (sInstance != null || sListener == null) return false;
        sListener.onObjectInitFailed(object);
        return true;
    }

    public static boolean reserveReflectMethod(ReflectMethod method) {
        if (sInstance != null || sListener == null) return false;
        sListener.onMethodCallMissed(method);
        return true;
    }

    private void initCore() {
        try {
            Class<?> clazz = getBridgeClass("XWalkCoreBridge");
            ReflectMethod method = new ReflectMethod(null,
                    clazz, "init", Context.class, Object.class);
            method.invoke(mBridgeContext, this);

            mCore = new ReflectMethod(null, clazz, "getInstance").invoke();
        } catch (RuntimeException e) {
            Assert.fail("initCore failed");
        }
    }

    private void resetCore() {
        try {
            ReflectMethod method = new ReflectMethod(null,
                    getBridgeClass("XWalkCoreBridge"), "reset", Object.class);
            method.invoke(mCore);
        } catch (RuntimeException e) {
            Assert.fail("resetCore failed");
        }
    }

    private XWalkCoreWrapper(int minSdkVersion) {
        mSdkVersion = XWalkSdkVersion.SDK_VERSION;
        mMinSdkVersion = (minSdkVersion > 0 && minSdkVersion <= mSdkVersion) ?
                minSdkVersion : mSdkVersion;
    }

    private boolean findEmbeddedCore() {
        mBridgeContext = null;
        mBridgeLoader = XWalkCoreWrapper.class.getClassLoader();
        if (!checkCoreArchitecture()) {
            mBridgeLoader = null;
            return false;
        }

        Log.d(TAG, "Running in embedded mode");
        return true;
    }

    private boolean findSharedCore() {
        XWalkApplication application = XWalkApplication.getApplication();
        if (application == null) Assert.fail("Must use or extend XWalkApplication");

        if (!XWalkSdkVersion.VERIFY_XWALK_APK) {
            Log.d(TAG, "Not verifying the package integrity of Crosswalk runtime library");
        } else {
            try {
                PackageInfo packageInfo = application.getPackageManager().getPackageInfo(
                        XWALK_APK_PACKAGE, PackageManager.GET_SIGNATURES);
                if (!verifyPackageInfo(packageInfo,
                        XWalkSdkVersion.XWALK_APK_HASH_ALGORITHM,
                        XWalkSdkVersion.XWALK_APK_HASH_CODE)) {
                    mCoreStatus = LibraryStatus.SIGNATURE_CHECK_ERROR;
                    return false;
                }
            } catch (NameNotFoundException e) {
                Log.d(TAG, "Crosswalk package not found");
                mCoreStatus = LibraryStatus.NOT_FOUND;
                return false;
            }
        }

        try {
            mBridgeContext = application.createPackageContext(XWALK_APK_PACKAGE,
                    Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);
        } catch (NameNotFoundException e) {
            Log.d(TAG, "Crosswalk package not found");
            mCoreStatus = LibraryStatus.NOT_FOUND;
            return false;
        }

        mBridgeLoader = mBridgeContext.getClassLoader();
        if (!checkCoreVersion() || !checkCoreArchitecture()) {
            mBridgeContext = null;
            mBridgeLoader = null;
            return false;
        }

        application.addResource(mBridgeContext.getResources());
        Log.d(TAG, "Running in shared mode");
        return true;
    }

    private boolean checkCoreVersion() {
        try {
            Class<?> clazz = getBridgeClass("XWalkCoreVersion");
            int libVersion = (int) new ReflectField(null, clazz, "LIB_VERSION").get();
            int minLibVersion = (int) new ReflectField(null, clazz, "MIN_LIB_VERSION").get();
            Log.d(TAG, "libVersion:" + libVersion + ", minLib:" + minLibVersion);
            Log.d(TAG, "sdkVersion:" + mSdkVersion + ", minSdk:" + mMinSdkVersion);
            Assert.assertTrue(libVersion >= minLibVersion);

            if (mMinSdkVersion > libVersion) {
                mCoreStatus = LibraryStatus.OLDER_VERSION;
            } else if (mSdkVersion < minLibVersion) {
                mCoreStatus = LibraryStatus.NEWER_VERSION;
            } else {
                mCoreStatus = LibraryStatus.MATCHED;
            }
        } catch (RuntimeException e) {
            Log.d(TAG, "Failed to check library version");
            mCoreStatus = LibraryStatus.NOT_FOUND;
            return false;
        }

        return true;
    }

    private boolean checkCoreArchitecture() {
        try {
            Class<?> clazz = getBridgeClass("XWalkViewDelegate");
            ReflectMethod method =
                    new ReflectMethod(null, clazz, "loadXWalkLibrary", Context.class);
            method.invoke(mBridgeContext);
            mCoreStatus = LibraryStatus.MATCHED;
        } catch (RuntimeException e) {
            Log.d(TAG, "Failed to load native library");
            mCoreStatus = LibraryStatus.NOT_FOUND;
            return false;
        }

        return true;
    }

    private boolean verifyPackageInfo(PackageInfo packageInfo,
            String hashAlgorithm, String hashCode) {
        if (packageInfo.signatures == null) {
            Log.e(TAG, "No signature in package info");
            return false;
        }

        MessageDigest md = null;
        try {
            md = MessageDigest.getInstance(hashAlgorithm);
        } catch (NoSuchAlgorithmException | NullPointerException e) {
            Assert.fail("Hash algorithm " + hashAlgorithm + " is not available");
        }

        byte[] hashArray = hexStringToByteArray(hashCode);
        if (hashArray == null) {
            Assert.fail("Hash code is invalid");
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
            return new ReflectMethod(null, object, "getBridge").invoke();
        } catch (RuntimeException e) {
        }
        return null;
    }

    public Object getWrapperObject(Object object) {
        try {
            return new ReflectMethod(null, object, "getWrapper").invoke();
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

    @Override
    public boolean handleException(RuntimeException exception) {
        if (sListener == null) return false;
        sListener.onXWalkLibraryRuntimeError(mCoreStatus, exception);
        return true;
    }
}
