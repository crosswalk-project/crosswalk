// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.content.Context;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

class XWalkCoreBridge {
    public static final int XWALK_API_VERSION = 1;
    public static final int XWALK_MIN_API_VERSION = 1;

    private static final String WRAPPER_PACKAGE = "org.xwalk.core";
    private static final String JAVASCRIPT_INTERFACE = "JavascriptInterface";

    private static final String BRIDGE_PACKAGE = "org.xwalk.core.internal";
    private static final String XWALK_CONTENT = "XWalkContent";

    private static XWalkCoreBridge instance;

    private Context mContext;
    private Object mWrapper;
    private ClassLoader mWrapperLoader;

    public static XWalkCoreBridge getInstance() {
        return instance;
    }

    public static void init(Context context, Object wrapper) {
        instance = new XWalkCoreBridge(context, wrapper);
    }

    public static void reset(Object object) {
        instance = (XWalkCoreBridge) object;
    }

    public XWalkCoreBridge(Context context, Object wrapper) {
        mContext = context;
        mWrapper = wrapper;
        mWrapperLoader = wrapper.getClass().getClassLoader();

        try {
            Class<?> javascriptInterface = mWrapperLoader.loadClass(
                    WRAPPER_PACKAGE + "." + JAVASCRIPT_INTERFACE);
            Class<?> xwalkContent = XWalkCoreBridge.class.getClassLoader().loadClass(
                    BRIDGE_PACKAGE + "." + XWALK_CONTENT);

            Method method = xwalkContent.getDeclaredMethod(
                    "setJavascriptInterfaceClass", javascriptInterface.getClass());
            method.invoke(null, javascriptInterface);
        } catch (ClassNotFoundException | NoSuchMethodException | NullPointerException |
                IllegalAccessException | IllegalArgumentException | InvocationTargetException e) {
            assert(false);
        }
    }

    public Context getContext() {
        return mContext;
    }

    public Object getBridge(Object object) {
        try {
            Method method = object.getClass().getDeclaredMethod("getBridge");
            method.setAccessible(true);
            Object ret = method.invoke(object);
            method.setAccessible(false);
            return ret;
        } catch (NoSuchMethodException | NullPointerException |
                IllegalAccessException | IllegalArgumentException | InvocationTargetException e) {
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

    public void handleException(Throwable throwable) {
        try {
            Method method = mWrapper.getClass().getMethod("handleException", Throwable.class);
            method.invoke(null, throwable);
        } catch (Exception e) {
            assert(false);
        }
    }
}
