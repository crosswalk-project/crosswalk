// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extension;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Annotated fields and method can be exposed to JavaScript.
 */
@Retention(RetentionPolicy.RUNTIME)
@Target({ElementType.METHOD})
public @interface JsConstructor {

    // The main Java class binding to the JS constructor.
    public Class<?> mainClass();

    // If the constructor is the entry point of this extension.
    // If true, jsName property will be ignored.
    public boolean isEntryPoint() default false;
}
