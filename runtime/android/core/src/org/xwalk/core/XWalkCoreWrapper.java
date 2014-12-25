// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.Context;
import android.content.pm.PackageManager.NameNotFoundException;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class XWalkCoreWrapper {
    public static final int XWALK_API_VERSION = 1;
    public static final String XWALK_CORE_PACKAGE = "org.xwalk.core";

    private static final String BRIDGE_PACKAGE = "org.xwalk.core.internal";
    private static final String XWALK_CORE_BRIDGE = "XWalkCoreBridge";
    private static final String XWALK_VIEW_DELEGATE = "XWalkViewDelegate";

    private static XWalkCoreListener listener;
    private static XWalkCoreWrapper instance;
    private static XWalkCoreWrapper provisionalInstance;

    private Object mBridge;
    private Context mBridgeContext;
    private ClassLoader mBridgeLoader;

    private int mMinAPIVersion;
    private XWalkCoreStatus mCoreStatus;

    public enum XWalkCoreStatus {
        MATCHED,
        NOT_FOUND,
        NEWER_VERSION,
        OLDER_VERSION
    }

    public interface XWalkCoreListener {
        public void onXWalkCoreReady();
        public void onXWalkCoreStartupError(Throwable e, XWalkCoreStatus status);
        public void onXWalkCoreRuntimeError(Throwable e, XWalkCoreStatus status);
        
        public void reserveReflectObject(Object object);
        public void reserveReflectMethod(ReflectMethod method);
    }

    public static XWalkCoreWrapper getInstance() {
        return instance;
    }

    public static void initEmbeddedMode() {
        reset(null, null);
        check();
        init();
        assert(instance.mCoreStatus == XWalkCoreStatus.MATCHED && !instance.isSharedMode());
    }

    public static void reset(XWalkCoreListener coreListener, XWalkCoreWrapper coreWrapper) {
        listener = coreListener;
        instance = coreWrapper;
        if (instance != null) instance.resetBridge();
    }

    public static void check() {
        check(-1);
    }

    public static void check(int minAPIVersion) {
        assert(instance == null);
        provisionalInstance = new XWalkCoreWrapper(minAPIVersion);
        if (listener == null) return;

        XWalkCoreStatus status = provisionalInstance.mCoreStatus;
        if (status == XWalkCoreStatus.MATCHED) {
            listener.onXWalkCoreReady();
        } else {
            Throwable error = null;
            if (status == XWalkCoreStatus.NOT_FOUND) {
                error = new UnsatisfiedLinkError("XWalk Core Not Found");
            } else {
                error = new VerifyError("API Incompatible");
            }
            listener.onXWalkCoreStartupError(error, status);
        }
    }

    public static void init() {
        instance = provisionalInstance;
        instance.initBridge();
    }

    public static boolean reserveReflectObject(Object object) {
        if (instance != null || listener == null) return false;
        listener.reserveReflectObject(object);
        return true;
    }

    public static boolean reserveReflectMethod(ReflectMethod method) {
        if (instance != null || listener == null) return false;
        listener.reserveReflectMethod(method);
        return true;
    }

    private XWalkCoreWrapper(int minAPIVersion) {
        if (minAPIVersion <= XWALK_API_VERSION) {
            mMinAPIVersion = minAPIVersion;
        } else {
            mMinAPIVersion = XWALK_API_VERSION;
        }
        
        if (!findEmbeddedBridge() && !findSharedBridge()) {
            mCoreStatus = XWalkCoreStatus.NOT_FOUND;
        }
    }

    private boolean findEmbeddedBridge() {
        try {
            mBridgeLoader = XWalkCoreWrapper.class.getClassLoader();
            
            Class<?> clazz = mBridgeLoader.loadClass(BRIDGE_PACKAGE + "." + XWALK_VIEW_DELEGATE);
            Method method = clazz.getMethod("loadXWalkLibrary", Context.class);
            method.invoke(null, (Context) null);

            mCoreStatus = XWalkCoreStatus.MATCHED;
        } catch (ClassNotFoundException | NoSuchMethodException | NullPointerException |
                IllegalAccessException | IllegalArgumentException | InvocationTargetException e) {
            mBridgeLoader = null;
            return false;
        }
        return true;
    }

    private boolean findSharedBridge() {
        try {
            XWalkApplication xwalkApp = XWalkApplication.getApplication();
            mBridgeContext = xwalkApp.createPackageContext(XWALK_CORE_PACKAGE,
                    Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);
            mBridgeLoader = mBridgeContext.getClassLoader();
            
            Class<?> clazz = mBridgeLoader.loadClass(BRIDGE_PACKAGE + "." + XWALK_CORE_BRIDGE);
            int coreVersion = (int) clazz.getField("XWALK_API_VERSION").get(null);
            int minCoreVersion = (int) clazz.getField("XWALK_MIN_API_VERSION").get(null);

            if (mMinAPIVersion > coreVersion) {
                mCoreStatus = XWalkCoreStatus.OLDER_VERSION;
            } else if (XWALK_API_VERSION < minCoreVersion) {
                mCoreStatus = XWalkCoreStatus.NEWER_VERSION;
            } else {
                mCoreStatus = XWalkCoreStatus.MATCHED;
            }

            xwalkApp.addResource(mBridgeContext.getResources());
        } catch (NameNotFoundException | ClassNotFoundException | NoSuchFieldException |
                IllegalAccessException | IllegalArgumentException | NullPointerException e) {
            mBridgeContext = null;
            mBridgeLoader = null;
            return false;
        }
        return true;
    }

    private void initBridge() {
        try {
            Class<?> clazz = mBridgeLoader.loadClass(BRIDGE_PACKAGE + "." + XWALK_CORE_BRIDGE);
            Method method = clazz.getMethod("init", Context.class, Object.class);
            method.invoke(null, mBridgeContext, this);

            method = clazz.getMethod("getInstance");
            mBridge = method.invoke(null);
        } catch (ClassNotFoundException | NoSuchMethodException | NullPointerException |
                IllegalAccessException | IllegalArgumentException | InvocationTargetException e) {
            assert(false);
        }
    }

    private void resetBridge() {
        try {
            Class<?> clazz = mBridgeLoader.loadClass(BRIDGE_PACKAGE + "." + XWALK_CORE_BRIDGE);
            Method method = clazz.getMethod("reset", Object.class);
            method.invoke(null, mBridge);
        } catch (ClassNotFoundException | NoSuchMethodException | NullPointerException |
                IllegalAccessException | IllegalArgumentException | InvocationTargetException e) {
            assert(false);
        }
    }

    public boolean isSharedMode() {
        return mBridgeContext != null;
    }

    public Object getWrapper(Object object) {
        try {
            Method method = object.getClass().getMethod("getWrapper");
            return method.invoke(object);
        } catch (NoSuchMethodException | NullPointerException |
                IllegalAccessException | IllegalArgumentException | InvocationTargetException e) {
        }
        return null;
    }

    public Object getBridge(Object object) {
        if (object == null) return null;

        for (Class<?> clazz = object.getClass(); clazz != null; clazz = clazz.getSuperclass()) {
            try {
                Method method = clazz.getDeclaredMethod("getBridge");
                method.setAccessible(true);
                return method.invoke(object);
            } catch (NoSuchMethodException |
                    IllegalAccessException | NullPointerException | IllegalArgumentException |
                    InvocationTargetException e) {
            }
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

    public void handleException(Throwable e) {
        if (listener != null) listener.onXWalkCoreRuntimeError(e, mCoreStatus);
    }
}
