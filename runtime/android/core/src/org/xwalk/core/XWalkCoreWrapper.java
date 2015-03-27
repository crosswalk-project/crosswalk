// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.Context;
import android.content.pm.PackageManager.NameNotFoundException;
import android.util.Log;

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
        if (!checkCoreVersion() || !checkCoreArchitecture()) {
            mBridgeLoader = null;
            return false;
        }

        Log.d(TAG, "Running on embedded mode");
        return true;
    }

    private boolean findSharedCore() {
        XWalkApplication application = XWalkApplication.getApplication();
        if (application == null) Assert.fail("Must use or extend XWalkApplication");

        try {
            mBridgeContext = application.createPackageContext(XWALK_APK_PACKAGE,
                    Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);
        } catch (NameNotFoundException e) {
            Log.d(TAG, "XWalk apk not found");
            return false;
        }

        mBridgeLoader = mBridgeContext.getClassLoader();
        if (!checkCoreVersion() || !checkCoreArchitecture()) {
            mBridgeContext = null;
            mBridgeLoader = null;
            return false;
        }

        Log.d(TAG, "Running on shared mode");
        application.addResource(mBridgeContext.getResources());
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
            mCoreStatus = LibraryStatus.NOT_FOUND;
        }

        return mCoreStatus != LibraryStatus.NOT_FOUND;
    }

    private boolean checkCoreArchitecture() {
        try {
            Class<?> clazz = getBridgeClass("XWalkViewDelegate");
            ReflectMethod method =
                    new ReflectMethod(null, clazz, "loadXWalkLibrary", Context.class);
            method.invoke(mBridgeContext);
        } catch (RuntimeException e) {
            Log.d(TAG, "Failed to load native library");
            mCoreStatus = LibraryStatus.NOT_FOUND;
            return false;
        }

        return true;
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
