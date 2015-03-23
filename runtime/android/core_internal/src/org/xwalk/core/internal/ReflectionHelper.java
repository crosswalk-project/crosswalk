// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.xwalk.core.internal;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.RandomAccessFile;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.security.cert.Certificate;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.HashMap;
import java.util.Set;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.zip.ZipEntry;

import dalvik.system.DexClassLoader;

import android.content.Context;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.Signature;
import android.widget.Toast;

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

    // We are doing following checks before loading shared library:
    //  1. The APK contains only one Certificate, this is to protect from FakeID attack.
    //  2. The contained certificate is exactly the one expected to be.
    //  3. additional check for master key exploit since Android 4.0.
    //     From http://androidvulnerabilities.org/by/manufacturer/all,
    //     There are three master key exploit since Android 4.0.
    public static boolean checkLibrary(Context app, Context library) throws NameNotFoundException, IOException {
        String apk = library.getPackageCodePath();
        if (apk == null || apk.isEmpty()) return false;
        JarFile apkFile = new JarFile(apk);
        Enumeration<JarEntry> entries = apkFile.entries();
        int rsaCount = 0;
        boolean rsaMatched = false;
        boolean hasDuplicateEntries = false;
        Set<String> entryNames = new HashSet<String>();
        while(entries.hasMoreElements()) {
            JarEntry entry = entries.nextElement();
            String entryName = entry.getName();
            if (entryNames.contains(entryName)) {
                hasDuplicateEntries = true;
                break;
            } else {
                entryNames.add(entryName);
            }
            if (entryName.toUpperCase().startsWith("META-INF/") && entryName.toUpperCase().endsWith(".RSA")) {
                rsaCount ++;
                InputStream is = apkFile.getInputStream(entry);
                byte[] tmpBuffer = new byte[1024];
                ByteArrayOutputStream buffer = new ByteArrayOutputStream();
                int count;
                while ((count = is.read(tmpBuffer, 0, 1024)) != -1) {
                    buffer.write(tmpBuffer, 0, count);
                }
                buffer.flush();
                is.close();
                Signature sig = new Signature(buffer.toByteArray());
                String sigContent = sig.toCharsString();
                // TODO(wang16): Compare with expected RSA
                if (sigContent.isEmpty()) continue;
                rsaMatched = true;
            }
        }
        apkFile.close();
        // Crosswalk Library apk should only contain one certificate.
        // Multiple signature introduces fake id risk.
        if (rsaCount != 1) return false;
        // For master key exploit 8219321, it's using multiple entries with same name to bypass system checking.
        // If there is no duplicate entries, we are safe.
        if (hasDuplicateEntries) return false;
        // For master key exploit 9695860, it's only applicable to the APKs with classes.dex smaller than 64k,
        // crosswalk library has classes.dex much more bigger than 64k. So no need to check for it.
        // For master key exploit 9950697, it's using different values for the file name length in centralDir and
        // local header to make native side load malicious content. Here we do additional check for the both
        // file name length of each entry to be safe.
        if (!checkFileNameLength(apk)) return false;
        return rsaMatched;
    }

    private static boolean checkFileNameLength(String apk) throws IOException {
        RandomAccessFile raf = new RandomAccessFile(apk, "r");
        // Following code are partly picked from AOSP.

        // Scan back, looking for the End Of Central Directory field. If the zip file doesn't
        // have an overall comment (unrelated to any per-entry comments), we'll hit the EOCD
        // on the first try.
        // No need to synchronize raf here -- we only do this when we first open the zip file.
        long scanOffset = raf.length() - 22; // ENDHDR
        if (scanOffset < 0) {
            // File too short to be a zip file
            raf.close();
            return false;
        }
        long stopOffset = scanOffset - 65536;
        if (stopOffset < 0) {
            stopOffset = 0;
        }
        final int ENDHEADERMAGIC = 0x06054b50;
        while (true) {
            raf.seek(scanOffset);
            if (Integer.reverseBytes(raf.readInt()) == ENDHEADERMAGIC) {
                break;
            }
            scanOffset--;
            if (scanOffset < stopOffset) {
                // EOCD not found; not a zip file?
                raf.close();
                return false;
            }
        }
        // Read the End Of Central Directory. ENDHDR includes the signature bytes,
        // which we've already read.
        int diskNumber = Short.reverseBytes(raf.readShort()) & 0xffff;
        int diskWithCentralDir = Short.reverseBytes(raf.readShort()) & 0xffff;
        int numEntries = Short.reverseBytes(raf.readShort()) & 0xffff;
        int totalNumEntries = Short.reverseBytes(raf.readShort()) & 0xffff;
        raf.skipBytes(4); // Ignore centralDirSize.
        long centralDirOffset = ((long) (Integer.reverseBytes(raf.readInt()))) & 0xffffffffL;
        int commentLength = Short.reverseBytes(raf.readShort()) & 0xffff;
        if (numEntries != totalNumEntries || diskNumber != 0 || diskWithCentralDir != 0) {
            // spanned archives not supported
            raf.close();
            return false;
        }
        if (commentLength > 0) {
            raf.skipBytes(commentLength);
        }
        // Seek to the first CDE and read all entries.
        // We have to do this now (from the constructor) rather than lazily because the
        // public API doesn't allow us to throw IOException except from the constructor
        // or from getInputStream.
        raf.seek(centralDirOffset);
        for (int i = 0; i < numEntries; ++i) {
            long entryOffsetInCentralDir = raf.getFilePointer();
            int sig = Integer.reverseBytes(raf.readInt());
            if (sig != 0x0000000002014b50) { // CENSIG
                 // Central Directory Entry not found
                raf.close();
                return false;
            }
            raf.seek(entryOffsetInCentralDir + 28);
            int nameLength = Short.reverseBytes(raf.readShort()) & 0xffff;
            int extraLength = Short.reverseBytes(raf.readShort()) & 0xffff;
            int commentByteCount = Short.reverseBytes(raf.readShort()) & 0xffff;
            raf.seek(entryOffsetInCentralDir + 42);
            long localHeaderRelOffset = ((long) (Integer.reverseBytes(raf.readInt()))) & 0xffffffffL;
            long savedOffset = raf.getFilePointer();
            raf.seek(localHeaderRelOffset + 26);
            int nameLengthInLocalHeader = Short.reverseBytes(raf.readShort()) & 0xffff;
            raf.seek(savedOffset);
            raf.skipBytes(nameLength + commentByteCount + extraLength);
            if (nameLengthInLocalHeader != nameLength) {
                raf.close();
                return false;
            }
        }
        raf.close();
        return true;
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
            DexClassLoader bridgeClasses = null;
            try {
                sBridgeContext = app.createPackageContext(LIBRARY_APK_PACKAGE, 0);
                if (!checkLibrary(app, sBridgeContext)) {
                    handleException("Can't verify the Crosswalk Library");
                    return;
                }
                bridgeClasses = new DexClassLoader(
                        sBridgeContext.getPackageCodePath(),
                        app.getCacheDir().getAbsolutePath(),
                        sBridgeContext.getApplicationInfo().nativeLibraryDir,
                        sBridgeContext.getClassLoader());
                sAlreadyUsingLibrary = true;
            } catch (PackageManager.NameNotFoundException e) {
                handleException(e);
            } catch (IOException e) {
                handleException(e);
            }
            if (sBridgeContext != null) {
                app.addResource(sBridgeContext.getResources());
                initClassLoader(bridgeClasses, sBridgeContext);
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
