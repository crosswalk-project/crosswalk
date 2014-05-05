// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Map;
import java.util.HashMap;

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
    private static boolean sIsWrapper;
    private final static String INTERNAL_PACKAGE = "org.xwalk.core.internal";

    public static void init(boolean crossPackage) {
        assert isWrapper();
        if (!crossPackage) {
            initClassLoader(ReflectionHelper.class.getClassLoader());
        } else {
            // TODO(wang16): Support shared mode and initClassLoader cross package.
        }
    }

    public static void initClassLoader(ClassLoader loader) {
        sBridgeOrWrapperLoader = loader;
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
                Method initInBridge = helperInBridge.getMethod("initClassLoader", ClassLoader.class);
                initInBridge.invoke(null, ReflectionHelper.class.getClassLoader());
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
        if (sBridgeOrWrapperLoader == null) init(false);
        try {
            return sBridgeOrWrapperLoader.loadClass(clazz);
        } catch (ClassNotFoundException e) {
            handleException(e);
            return null;
        }
    }

    public static Method loadMethod(Class<?> clazz, String name, Object... paramTypes) {
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
        if (instance == null) return null;
        Class<?> clazz = instance.getClass();
        Method method = sBridgeWrapperMap.get(clazz);
        if (method == null) {
            String methodName = "getBridge";
            if (sIsWrapper) {
                methodName = "getWrapper";
            }
            try {
                method = clazz.getMethod(methodName);
            } catch (NoSuchMethodException e) {
                handleException(e);
            }
            if (method != null) sBridgeWrapperMap.put(clazz, method);
        }
        return invokeMethod(method, instance);
    }

    private static boolean isWrapper() {
        return !ReflectionHelper.class.getPackage().getName().equals(INTERNAL_PACKAGE);
    }

    static {
        sIsWrapper = isWrapper();
    }
}
