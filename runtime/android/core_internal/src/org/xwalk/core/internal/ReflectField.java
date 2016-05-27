// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import java.lang.reflect.Field;
import java.util.concurrent.RejectedExecutionException;

class ReflectField {
    private Object mInstance;
    private Class<?> mClass;
    private String mName;
    private Field mField;

    public ReflectField() {
    }

    public ReflectField(Object instance, String name) {
        init(instance, null, name);
    }

    public ReflectField(Class<?> clazz, String name) {
        init(null, clazz, name);
    }

    public boolean init(Object instance, Class<?> clazz, String name) {
        mInstance = instance;
        mClass = clazz != null ? clazz : (instance != null ? instance.getClass() : null);
        mName = name;
        mField = null;

        if (mClass == null) return false;

        try {
            mField = mClass.getField(mName);
        } catch (NoSuchFieldException e) {
            for (Class<?> parent = mClass; parent != null; parent = parent.getSuperclass()) {
                try {
                    mField = parent.getDeclaredField(mName);
                    mField.setAccessible(true);
                    break;
                } catch (NoSuchFieldException e2) {
                }
            }
        }
        return mField != null;
    }

    public Object get() {
        if (mField == null) {
            throw new UnsupportedOperationException(toString());
        }

        try {
            return mField.get(mInstance);
        } catch (IllegalAccessException | NullPointerException e) {
            throw new RejectedExecutionException(e);
        } catch (IllegalArgumentException e) {
            throw e;
        } catch (ExceptionInInitializerError e) {
            throw new RuntimeException(e);
        }
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
}
