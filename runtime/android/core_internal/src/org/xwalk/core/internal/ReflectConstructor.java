// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.concurrent.RejectedExecutionException;

class ReflectConstructor {
    private ReflectExceptionHandler mHandler;

    private Class<?> mClass;
    private Class<?>[] mParameterTypes;
    private Constructor<?> mConstructor;

    public ReflectConstructor() {
    }

    public ReflectConstructor(ReflectExceptionHandler handler,
            Class<?> clazz, Class<?>... parameterTypes) {
        init(handler, clazz, parameterTypes);
    }

    public boolean init(ReflectExceptionHandler handler,
            Class<?> clazz, Class<?>... parameterTypes) {
        mHandler = handler;
        mClass = clazz;
        mParameterTypes = parameterTypes;
        mConstructor = null;

        if (mClass == null) return false;

        try {
            mConstructor = mClass.getConstructor(mParameterTypes);
        } catch (NoSuchMethodException e) {
            try {
                mConstructor = mClass.getDeclaredConstructor(mParameterTypes);
                mConstructor.setAccessible(true);
            } catch (NoSuchMethodException e2) {
            }
        }
        return mConstructor != null;
    }

    public Object newInstance(Object... args) {
        if (mConstructor == null) {
            handleException(new UnsupportedOperationException(toString()));
            return null;
        }

        try {
            return mConstructor.newInstance(args);
        } catch (IllegalAccessException | InstantiationException e) {
            handleException(new RejectedExecutionException(e));
        } catch (IllegalArgumentException e) {
            handleException(e);
        } catch (InvocationTargetException e) {
            handleException(new RuntimeException(e.getCause()));
        }
        return null;
    }

    public boolean isNull() {
        return mConstructor == null;
    }

    public String toString() {
        if (mConstructor != null) return mConstructor.toString();

        String ret = "";
        if (mClass != null) ret += mClass.toString();
        return ret;
    }

    private void handleException(RuntimeException exception) {
        if (mHandler == null || !mHandler.handleException(exception)) {
            throw exception;
        }
    }
}
