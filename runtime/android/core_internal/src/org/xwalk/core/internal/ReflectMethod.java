// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.concurrent.RejectedExecutionException;

class ReflectMethod {
    private ReflectExceptionHandler mHandler;

    private Object mInstance;
    private Class<?> mClass;
    private String mName;
    private Class<?>[] mParameterTypes;
    private Method mMethod;
    private Object[] mArguments;

    public ReflectMethod() {
    }

    public ReflectMethod(ReflectExceptionHandler handler,
            Object instance, String name, Class<?>... parameterTypes) {
        init(handler, instance, null, name, parameterTypes);
    }

    public ReflectMethod(ReflectExceptionHandler handler,
            Class<?> clazz, String name, Class<?>... parameterTypes) {
        init(handler, null, clazz, name, parameterTypes);
    }

    public boolean init(ReflectExceptionHandler handler,
            Object instance, Class<?> clazz, String name, Class<?>... parameterTypes) {
        mHandler = handler;
        mInstance = instance;
        mClass = clazz != null ? clazz : (instance != null ? instance.getClass() : null);
        mName = name;
        mParameterTypes = parameterTypes;
        mMethod = null;

        if (mClass == null) return false;

        try {
            mMethod = mClass.getMethod(mName, mParameterTypes);
        } catch (NoSuchMethodException e) {
            for (Class<?> parent = mClass; parent != null; parent = parent.getSuperclass()) {
                try {
                    mMethod = parent.getDeclaredMethod(mName, mParameterTypes);
                    mMethod.setAccessible(true);
                    break;
                } catch (NoSuchMethodException e2) {
                }
            }
        }

        return mMethod != null;
    }

    public Object invoke(Object... args) {
        if (mMethod == null) {
            handleException(new UnsupportedOperationException(toString()));
            return null;
        }

        try {
            return mMethod.invoke(mInstance, args);
        } catch (IllegalAccessException | NullPointerException e) {
            handleException(new RejectedExecutionException(e));
        } catch (IllegalArgumentException e) {
            handleException(e);
        } catch (InvocationTargetException e) {
            handleException(new RuntimeException(e.getCause()));
        }
        return null;
    }

    public boolean isNull() {
        return mMethod == null;
    }

    public String toString() {
        if (mMethod != null) return mMethod.toString();

        String ret = "";
        if (mClass != null) ret += mClass.toString() + ".";
        if (mName != null) ret += mName;
        return ret;
    }

    public String getName() {
        return mName;
    }

    public Object getInstance() {
        return mInstance;
    }

    public Object[] getArguments() {
        return mArguments;
    }

    public void setArguments(Object... args) {
        mArguments = args;
    }

    public Object invokeWithArguments() {
        return invoke(mArguments);
    }

    private void handleException(RuntimeException exception) {
        if (mHandler == null || !mHandler.handleException(exception)) {
            throw exception;
        }
    }
}
