// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app.runtime;

import android.content.Context;
import android.content.pm.PackageManager.NameNotFoundException;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public abstract class CrossPackageWrapper {
    public final static String LIBRARY_APK_PACKAGE_NAME = "org.xwalk.runtime.lib";
    private Context mLibCtx;
    private Class<?> mTargetClass;
    private Constructor<?> mCreator;
    private CrossPackageWrapperExceptionHandler mExceptionHandler;
    private static boolean sLibraryEmbedded = true;

    public CrossPackageWrapper(Context ctx, String className,
            CrossPackageWrapperExceptionHandler handler, Class<?>... parameters) {
        try {
            mTargetClass = ctx.getClassLoader().loadClass(className);
            sLibraryEmbedded = true;
        } catch (ClassNotFoundException e) {
            sLibraryEmbedded = false;
        }
        mExceptionHandler = handler;
        try {
            if (sLibraryEmbedded) {
                mLibCtx = ctx;
            } else {
                mLibCtx = ctx.createPackageContext(
                        LIBRARY_APK_PACKAGE_NAME,
                        Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);
                mTargetClass =
                        mLibCtx.getClassLoader().loadClass(className);
            }
            mCreator = mTargetClass.getConstructor(parameters);
        } catch (NameNotFoundException e) {
            handleException(e);
        } catch (ClassNotFoundException e) {
            handleException(e);
        } catch (NoSuchMethodException e) {
            handleException(e);
        }
    }

    public Object createInstance(Object... parameters) {
        Object ret = null;
        if (mCreator != null) {
            try {
                ret = mCreator.newInstance(parameters);
            } catch (IllegalArgumentException e) {
                handleException(e);
            } catch (InstantiationException e) {
                handleException(e);
            } catch (IllegalAccessException e) {
                handleException(e);
            } catch (InvocationTargetException e) {
                handleException(e);
            }
        }
        return ret;
    }

    public void handleException(Exception e) {
        if (mExceptionHandler != null) mExceptionHandler.onException(e);
    }

    public void handleException(String e) {
        if (mExceptionHandler != null) mExceptionHandler.onException(e);
    }

    public Class<?> getTargetClass() {
        return mTargetClass;
    }

    public Context getLibraryContext() {
        return mLibCtx;
    }

    public Method lookupMethod(String method, Class<?>... parameters) {
        if (mTargetClass == null) return null;
        try {
            return mTargetClass.getMethod(method, parameters);
        } catch (NoSuchMethodException e) {
            handleException(e);
        }
        return null;
    }

    public Object invokeMethod(Method m, Object instance, Object... parameters) {
        Object ret = null;
        if (m != null) {
            try {
                ret = m.invoke(instance, parameters);
            } catch (IllegalArgumentException e) {
                handleException(e);
            } catch (IllegalAccessException e) {
                handleException(e);
            } catch (InvocationTargetException e) {
                handleException(e);
            } catch (NullPointerException e) {
                handleException(e);
            }
        }
        return ret;
    }

    public static boolean libraryIsEmbedded() {
        return sLibraryEmbedded;
    }
}
