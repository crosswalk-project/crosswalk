// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.Context;
import android.util.Log;

import java.lang.reflect.Field;

public class XWalkInternalResources {
    private static final String TAG = "XWalkInternalResources";
    
    private static boolean loaded = false;
    private final static String INTERNAL_RESOURCE_CLASSES[] = {
        "org.chromium.content.R",
        "org.chromium.ui.R"
    };
    private final static String GENERATED_RESOURCE_CLASS = "org.xwalk.core.R";

    // Doing org.chromium.content.R.<class>.<name> = org.xwalk.core.R.<class>.<name>
    // Use reflection to iterate over the target class is to avoid hardcode.
    private static void doResetIds(Context context) {
        ClassLoader classLoader = context.getClassLoader();
        for (String resourceClass : INTERNAL_RESOURCE_CLASSES) {
            try {
                Class<?> internalResource = classLoader.loadClass(resourceClass);
                Class<?>[] innerClazzs = internalResource.getClasses();
                for (Class<?> innerClazz : innerClazzs) {
                    Class<?> generatedInnerClazz;
                    String generatedInnerClassName = innerClazz.getName().replace(
                            resourceClass, GENERATED_RESOURCE_CLASS);
                    try {
                        generatedInnerClazz = classLoader.loadClass(generatedInnerClassName);
                    } catch (ClassNotFoundException e) {
                        Log.w(TAG, generatedInnerClassName + "is not found.");
                        continue;
                    }
                    Field[] fields = innerClazz.getFields();
                    for (Field field : fields) {
                        // It's final means we are probably not used as library project.
                        if (!field.isAccessible()) continue;
                        try {
                            int value = generatedInnerClazz.getField(field.getName()).getInt(null);
                            field.setInt(null, value);
                        } catch (IllegalAccessException e) {
                            Log.w(TAG, generatedInnerClazz.getName() + "." +
                                    field.getName() + " is not accessable.");
                        } catch (IllegalArgumentException e) {
                            Log.w(TAG, generatedInnerClazz.getName() + "." + 
                                    field.getName() + " is not int.");
                        } catch (NoSuchFieldException e) {
                            Log.w(TAG, generatedInnerClazz.getName() + "." + 
                                    field.getName() + " is not found.");
                        }
                    }
                }
            } catch (ClassNotFoundException e) {
                Log.w(TAG, resourceClass + "is not found.");
            }
        }
    }

    static void resetIds(Context context) {
        if (!loaded) {
            doResetIds(context);
            loaded = true;
        }
    }
}
