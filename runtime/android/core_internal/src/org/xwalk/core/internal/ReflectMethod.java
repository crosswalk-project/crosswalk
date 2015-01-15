// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.concurrent.RejectedExecutionException;

public class ReflectMethod {
    private Object handler;
    private Object instance;
    private String name;
    private Class<?>[] parameterTypes;
    private Class<?> clazz;
    private Method method;
    private Object[] arguments;

    public ReflectMethod() {
    }

    public ReflectMethod(Object handler, Object instance, String name, Class<?>... parameterTypes) {
        init(handler, instance, name, parameterTypes);
    }

    public ReflectMethod(Object handler, Class<?> clazz, String name, Class<?>... parameterTypes) {
        init(handler, clazz, name, parameterTypes);
    }

    public void init(Object handler, Object instance, String name, Class<?>... parameterTypes) {
        this.handler = handler;
        this.instance = instance;
        this.name = name;
        this.parameterTypes = parameterTypes;

        clazz = instance == null ? null : instance.getClass();
        try {
            method = clazz.getMethod(name, parameterTypes);
        } catch (NullPointerException | NoSuchMethodException e) {
            method = null;
        }
    }

    public void init(Object handler, Class<?> clazz, String name, Class<?>... parameterTypes) {
        this.handler = handler;
        this.name = name;
        this.parameterTypes = parameterTypes;
        this.clazz = clazz;

        try {
            method = clazz.getMethod(name, parameterTypes);
        } catch (NullPointerException | NoSuchMethodException e) {
            method = null;
        }
    }

    public Object invoke(Object... args) {
        if (method == null) {
            handleException(new UnsupportedOperationException(toString()));
            return null;
        }

        try {
            return method.invoke(instance, args);
        } catch (IllegalAccessException | IllegalArgumentException | NullPointerException e) {
            handleException(new RejectedExecutionException(toString()));
        } catch (InvocationTargetException e) {
            handleException(e);
        }
        return null;
    }

    public Object invokeWithReservedArguments() {
        return invoke(arguments);
    }

    public void reserveArguments(Object... args) {
        arguments = args;
    }

    public boolean isNull() {
        return method == null;
    }

    public String toString() {
        if (method != null) return method.toString();
        
        String ret = null;
        if (clazz != null) {
            ret += clazz.toString();
            if (name != null) ret += "." + name;
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
