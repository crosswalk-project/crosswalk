// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.concurrent.RejectedExecutionException;

class ReflectConstructor {
    private Class<?> mClass;
    private Class<?>[] mParameterTypes;
    private Constructor<?> mConstructor;

    public ReflectConstructor() {
    }

    public ReflectConstructor(Class<?> clazz, Class<?>... parameterTypes) {
        init(clazz, parameterTypes);
    }

    public boolean init(Class<?> clazz, Class<?>... parameterTypes) {
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
            throw new UnsupportedOperationException(toString());
        }

        try {
            return mConstructor.newInstance(args);
        } catch (IllegalAccessException | InstantiationException e) {
            throw new RejectedExecutionException(e);
        } catch (IllegalArgumentException e) {
            throw e;
        } catch (InvocationTargetException e) {
            throw new RuntimeException(e.getCause());
        }
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
}
