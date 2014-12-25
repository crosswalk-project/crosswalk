// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.concurrent.RejectedExecutionException;

public class ReflectConstructor {
    private Object handler;
    private Class<?> clazz;
    private Class<?>[] parameterTypes;
    private Constructor<?> constructor;

    public ReflectConstructor() {
    }

    public ReflectConstructor(Object handler, Class<?> clazz, Class<?>... parameterTypes) {
        this.handler = handler;
        this.clazz = clazz;
        this.parameterTypes = parameterTypes;

        try {
            constructor = clazz.getConstructor(parameterTypes);
        } catch (NullPointerException | NoSuchMethodException e) {
            constructor = null;
        }
    }

    public Object newInstance(Object... args) {
        if (constructor == null) {
            handleException(new UnsupportedOperationException(toString()));
            return null;
        }

        try {
            return constructor.newInstance(args);
        } catch (IllegalAccessException | IllegalArgumentException | InstantiationException e) {
            handleException(new RejectedExecutionException(toString()));
        } catch (InvocationTargetException e) {
            handleException(e);
        }
        return null;
    }

    public boolean isNull() {
        return constructor == null;
    }

    public String toString() {
        if (constructor != null) return constructor.toString();
        
        String ret = null;
        if (clazz != null) {
            ret += clazz.toString();
        }
        return ret;
    }

    private void handleException(Throwable throwable) {
        try {
            Method method = handler.getClass().getMethod("handleException", Throwable.class);
            method.invoke(handler, throwable);
        } catch (NullPointerException | NoSuchMethodException |
                IllegalArgumentException | IllegalAccessException | InvocationTargetException e) {
            assert(false);
        }
    }
}
