// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import java.lang.reflect.Field;
import java.util.concurrent.RejectedExecutionException;

class ReflectField {
    private ReflectExceptionHandler mHandler;

    private Object mInstance;
    private Class<?> mClass;
    private String mName;
    private Field mField;

    public ReflectField() {
    }

    public ReflectField(ReflectExceptionHandler handler, Object instance, String name) {
        init(handler, instance, null, name);
    }

    public ReflectField(ReflectExceptionHandler handler, Class<?> clazz, String name) {
        init(handler, null, clazz, name);
    }

    public boolean init(ReflectExceptionHandler handler,
            Object instance, Class<?> clazz, String name) {
        mHandler = handler;
        mInstance = instance;
        mClass = clazz != null ? clazz : (instance != null ? instance.getClass() : null);
        mName = name;
        mField = null;

        if (mClass == null) return false;

        try {
            mField = mClass.getField(mName);
        } catch (NoSuchFieldException e) {
            try {
                mField = mClass.getDeclaredField(mName);
                mField.setAccessible(true);
            } catch (NoSuchFieldException e2) {
            }
        }
        return mField != null;
    }

    public Object get() {
        if (mField == null) {
            handleException(new UnsupportedOperationException(toString()));
            return null;
        }

        try {
            return mField.get(mInstance);
        } catch (IllegalAccessException | NullPointerException e) {
            handleException(new RejectedExecutionException(e));
        } catch (IllegalArgumentException e) {
            handleException(e);
        } catch (ExceptionInInitializerError e) {
            handleException(new RuntimeException(e));
        }
        return null;
    }

    public boolean isNull() {
        return mField == null;
    }

    public String toString() {
        if (mField != null) return mField.toString();

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

    private void handleException(RuntimeException exception) {
        if (mHandler == null || !mHandler.handleException(exception)) {
            throw exception;
        }
    }
}
