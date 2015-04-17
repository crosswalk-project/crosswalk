// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.content.Context;

class XWalkCoreBridge {
    private static final String WRAPPER_PACKAGE = "org.xwalk.core";
    private static final String BRIDGE_PACKAGE = "org.xwalk.core.internal";

    private static XWalkCoreBridge sInstance;

    private Context mBridgeContext;
    private ClassLoader mWrapperLoader;

    public static XWalkCoreBridge getInstance() {
        return sInstance;
    }

    public static void init(Context context, Object wrapper) {
        sInstance = new XWalkCoreBridge(context, wrapper);
    }

    private XWalkCoreBridge(Context context, Object wrapper) {
        mBridgeContext = context;
        mWrapperLoader = wrapper.getClass().getClassLoader();

        Class<?> xwalkContent = getBridgeClass("XWalkContent");
        Class<?> javascriptInterface = getWrapperClass("JavascriptInterface");
        ReflectMethod method = new ReflectMethod(xwalkContent,
                "setJavascriptInterfaceClass", javascriptInterface.getClass());
        method.invoke(javascriptInterface);
    }

    public Context getContext() {
        return mBridgeContext;
    }

    public Object getBridgeObject(Object object) {
        try {
            return new ReflectMethod(object, "getBridge").invoke();
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
            return XWalkCoreBridge.class.getClassLoader().loadClass(BRIDGE_PACKAGE + "." + name);
        } catch (ClassNotFoundException e) {
        }
        return null;
    }
}
