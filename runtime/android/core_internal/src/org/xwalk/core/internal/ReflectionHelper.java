// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Map;
import java.util.HashMap;

import android.content.Context;
import android.content.pm.PackageManager;

/**
 * This class is used to encapsulate the reflection invoking for bridge and wrapper.
 *
 * org.xwalk.core.internal.ReflectionHelper is for bridge to use. A copy will be
 * made to org.xwalk.core.ReflectionHelper at build time for wrapper to use.
 */
public class ReflectionHelper {
    /**
     * This class contains the identical information of a constructor.
     *
     * Constructor of bridge and wrapper class will be initialized in the
     * static block.
     * It might happen before the RefletionHelper itself being initialized.
     * For those cases, record the information of the Constructor and load
     * them after ReflectionHelper initialized.
     */
    static class ConstructorHelper {
        private String fullClassName;
        private Object[] paramTypes;

        Constructor<?> loadConstructor() {
            Class<?> clazz = loadClass(fullClassName);
            if (clazz == null) return null;
            Class<?>[] params = new Class<?>[paramTypes.length];
            for (int i = 0; i < paramTypes.length; i++) {
                Object type = paramTypes[i];
                // paramTypes can be string or Class<?>, if it's string,
                // it means it's not in bridge/wrapper's classLoader,
                // we need to load it from wrapper/bridge's loader.
                if (type instanceof Class<?>) {
                    params[i] = (Class<?>) type;
                } else if (type instanceof String) {
                    params[i] = loadClass((String) type);
                }
            }
            try {
                return clazz.getConstructor(params);
            } catch (NoSuchMethodException e) {
                ReflectionHelper.handleException(e);
                return null;
            }
        }

        ConstructorHelper(String className, Object... paramTypes) {
            this.fullClassName = className;
            this.paramTypes = paramTypes;
        }
    }

    private static Map<Class<?>, Method> sBridgeWrapperMap = new HashMap<Class<?>, Method>();
    private static Map<String, Constructor<?>> sConstructorMap = new HashMap<String, Constructor<?>>();
    private static Map<String, ConstructorHelper> sConstructorHelperMap =
            new HashMap<String, ConstructorHelper>();
    private static ClassLoader sBridgeOrWrapperLoader = null;
    private static Context sBridgeContext = null;
    private static boolean sIsWrapper;
    private final static String INTERNAL_PACKAGE = "org.xwalk.core.internal";
    private final static String LIBRARY_APK_PACKAGE = "org.xwalk.core";
    /* Wrapper Only
    private static boolean sAllowCrossPackage = false;
    private static boolean sAlreadyUsingLibrary = false;
    private static SharedXWalkExceptionHandler sExceptionHandler = null;

    static void setExceptionHandler(SharedXWalkExceptionHandler handler) {
        sExceptionHandler = handler;
    }

    static boolean isUsingLibrary() {
        return sAlreadyUsingLibrary;
    }

    static boolean shouldUseLibrary() {
        if (sAlreadyUsingLibrary) return true;

        // TODO(wang16): There are many other conditions here.
        // e.g. Whether application uses the ApplicationClass we provided,
        //      Whether native library arch is correct.
        assert isWrapper();
        Class<?> delegateClass = null;
        try {
            ClassLoader classLoader = ReflectionHelper.class.getClassLoader();
            delegateClass = classLoader.loadClass(
                    INTERNAL_PACKAGE + "." + "XWalkViewDelegate");
        } catch (ClassNotFoundException e) {
            return true;
        }
        if (delegateClass == null) return true;
        try {
            Method loadXWalkLibrary = delegateClass.getDeclaredMethod(
                    "loadXWalkLibrary", Context.class);
            loadXWalkLibrary.invoke(null, (Context)null);
        } catch (NoSuchMethodException e) {
            return true;
        } catch (IllegalArgumentException e) {
            return true;
        } catch (IllegalAccessException e) {
            return true;
        } catch (InvocationTargetException e) {
            return true;
        } catch (UnsatisfiedLinkError e) {
            return true;
        }
        return false;
    }
    Wrapper Only */

    public static Context getBridgeContext() {
        return sBridgeContext;
    }

    /* Wrapper Only
    public static void allowCrossPackage() {
        sAllowCrossPackage = true;
    }
    Wrapper Only */

    public static void init() {
        assert isWrapper();
        /* Wrapper Only
        if (shouldUseLibrary()) {
            if (!sAllowCrossPackage) {
                handleException("Use SharedXWalkView if you want to support shared mode");
            }
            XWalkApplication app = XWalkApplication.getApplication();
            if (app == null) {
                // TODO(wang16): Handle this well.
                handleException("Shared mode requires XWalkApplication");
                return;
            }
            try {
                sBridgeContext = app.createPackageContext(
                        LIBRARY_APK_PACKAGE,
                        Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);
                sAlreadyUsingLibrary = true;
            } catch (PackageManager.NameNotFoundException e) {
                handleException(e);
            }
            if (sBridgeContext != null) {
                app.addResource(sBridgeContext.getResources());
                initClassLoader(sBridgeContext.getClassLoader(), sBridgeContext);
            }
        } else {
            initClassLoader(ReflectionHelper.class.getClassLoader(), null);
        }
        Wrapper Only */
    }

    public static void initClassLoader(ClassLoader loader, Context bridgeContext) {
        sBridgeOrWrapperLoader = loader;
        sBridgeContext = bridgeContext;
        sBridgeWrapperMap.clear();
        sConstructorMap.clear();
        try {
            for (String name : sConstructorHelperMap.keySet()) {
                ConstructorHelper helper = sConstructorHelperMap.get(name);
                if (helper != null) sConstructorMap.put(name, helper.loadConstructor());
            }
            if (sIsWrapper) {
                // Load the helper in bridge side and invoke the initClassLoader method of it
                // with wrapper's classloader via reflection.
                Class<?> helperInBridge =
                        sBridgeOrWrapperLoader.loadClass(INTERNAL_PACKAGE + "." + "ReflectionHelper");
                Method initInBridge = helperInBridge.getMethod(
                        "initClassLoader", ClassLoader.class, Context.class);
                initInBridge.invoke(null, ReflectionHelper.class.getClassLoader(), sBridgeContext);
            } else {
                // JavascriptInterface is an annotation class bridge will use but declared in
                // wrapper.
                Class<?> javascriptInterface =
                        sBridgeOrWrapperLoader.loadClass("org.xwalk.core.JavascriptInterface");
                Class<?> xwalkContentInInternal =
                        ReflectionHelper.class.getClassLoader().loadClass(
                                INTERNAL_PACKAGE + "." + "XWalkContent");
                Method setJavascriptInterface = xwalkContentInInternal.getDeclaredMethod(
                        "setJavascriptInterfaceClass", javascriptInterface.getClass());
                setJavascriptInterface.invoke(null, javascriptInterface);
            }
        } catch (Exception e) {
            handleException(e);
        }
    }

    public static void registerConstructor(String name, String clazz, Object... params) {
        sConstructorHelperMap.put(name, new ConstructorHelper(clazz, params));
    }

    public static Class<?> loadClass(String clazz) {
        // Any embedder using Embedding API should only use the exposed APIs which are
        // in wrapper, so the initialization process is always starting from wrapper.
        if (sBridgeOrWrapperLoader == null) init();
        if (sBridgeOrWrapperLoader == null) return null;
        try {
            return sBridgeOrWrapperLoader.loadClass(clazz);
        } catch (ClassNotFoundException e) {
            handleException(e);
            return null;
        }
    }

    public static Method loadMethod(Class<?> clazz, String name, Object... paramTypes) {
        if (sBridgeOrWrapperLoader == null) return null;
        Class<?>[] params = new Class<?>[paramTypes.length];
        for (int i = 0; i < paramTypes.length; i++) {
            Object type = paramTypes[i];
            if (type instanceof Class<?>) {
                params[i] = (Class<?>) type;
            } else if (type instanceof String) {
                params[i] = loadClass((String) type);
            }
        }
        try {
            return clazz.getMethod(name, params);
        } catch (NoSuchMethodException e) {
            handleException(e);
            return null;
        }
    }

    public static void handleException(Exception e) {
        e.printStackTrace();
        /* Wrapper Only
        if (isWrapper() && sExceptionHandler != null) {
            if (sExceptionHandler.handleException(e)) return;
        }
        Wrapper Only */
        throw new RuntimeException(e);
    }

    public static void handleException(String e) {
        handleException(new RuntimeException(e));
    }

    public static Object createInstance(String name, Object... parameters) {
        Object ret = null;
        Constructor<?> creator = sConstructorMap.get(name);
        if (creator == null) {
            ConstructorHelper helper = sConstructorHelperMap.get(name);
            if (helper != null) {
                creator = helper.loadConstructor();
                sConstructorMap.put(name, creator);
            }
        }
        if (creator != null) {
            try {
                ret = creator.newInstance(parameters);
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

    public static Object invokeMethod(Method m, Object instance, Object... parameters) {
        if (sBridgeOrWrapperLoader == null) return null;
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

    // Convert between wrapper and bridge instance.
    public static Object getBridgeOrWrapper(Object instance) {
        if (sBridgeOrWrapperLoader == null) return null;
        if (instance == null) return null;
        Class<?> clazz = instance.getClass();
        Method method = sBridgeWrapperMap.get(clazz);
        if (method == null) {
            String methodName = "getBridge";
            if (sIsWrapper) {
                methodName = "getWrapper";
            }
            try {
                method = clazz.getDeclaredMethod(methodName);
            } catch (NoSuchMethodException e) {
                handleException(e);
            }

            if (method == null)  {
                return invokeMethod(method, instance);
            } else {
                sBridgeWrapperMap.put(clazz, method);
            }
        }

        if (method.isAccessible()) return invokeMethod(method, instance);

        // This is to enable the accessibility of getBridge temporarily.
        // It's not public for documentation generating.
        method.setAccessible(true);
        Object ret = invokeMethod(method, instance);
        method.setAccessible(false);
        return ret;
    }

    private static boolean isWrapper() {
        return !ReflectionHelper.class.getPackage().getName().equals(INTERNAL_PACKAGE);
    }

    static {
        sIsWrapper = isWrapper();
    }
}
