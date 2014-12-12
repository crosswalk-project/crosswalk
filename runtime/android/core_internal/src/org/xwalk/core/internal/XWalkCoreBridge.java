// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.content.Context;

import junit.framework.Assert;

class XWalkCoreBridge implements ReflectExceptionHandler {
    public static final String WRAPPER_PACKAGE = "org.xwalk.core";
    public static final String BRIDGE_PACKAGE = "org.xwalk.core.internal";

    private static XWalkCoreBridge sInstance;

    private Context mContext;
    private Object mWrapper;
    private ClassLoader mWrapperLoader;
    private ClassLoader mBridgeLoader;

    public static XWalkCoreBridge getInstance() {
        return sInstance;
    }

    public static void init(Context context, Object wrapper) {
        sInstance = new XWalkCoreBridge(context, wrapper);
    }

    public static void reset(Object object) {
        sInstance = (XWalkCoreBridge) object;
    }

    private XWalkCoreBridge(Context context, Object wrapper) {
        mContext = context;
        mWrapper = wrapper;
        mWrapperLoader = wrapper.getClass().getClassLoader();
        mBridgeLoader = XWalkCoreBridge.class.getClassLoader();
        if (context != null) mBridgeLoader = context.getClassLoader();

        try {
            Class<?> javascriptInterface = getWrapperClass("JavascriptInterface");
            Class<?> xwalkContent = getBridgeClass("XWalkContent");

            ReflectMethod method = new ReflectMethod(null, xwalkContent,
                    "setJavascriptInterfaceClass", javascriptInterface.getClass());
            method.invoke(javascriptInterface);
        } catch (RuntimeException e) {
            Assert.fail("setJavascriptInterfaceClass failed");
        }
    }

    public Context getContext() {
        return mContext;
    }

    public Object getBridgeObject(Object object) {
        try {
            return new ReflectMethod(null, object, "getBridge").invoke();
        } catch (RuntimeException e) {
        }
        return null;
    }

    public Class<?> getWrapperClass(String name) {
        try {
            return mWrapperLoader.loadClass(WRAPPER_PACKAGE + "." + name);
        } catch (ClassNotFoundException e) {
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
        try {
            ReflectMethod method = new ReflectMethod(null,
                    mWrapper, "handleException", RuntimeException.class);
            return (boolean) method.invoke(exception);
        } catch (RuntimeException e) {
        }
        return false;
    }
}
