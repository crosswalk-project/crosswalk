// Copyright 2016 The Intel Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import org.chromium.base.annotations.JNINamespace;

/**
 * Exposes a subset of Chromium form database to XWalkView database for managing autocomplete
 * functionality.
 */
@JNINamespace("xwalk")
public class XWalkFormDatabase {

    public static boolean hasFormData() {
        return nativeHasFormData();
    }

    public static void clearFormData() {
        nativeClearFormData();
    }

    //--------------------------------------------------------------------------------------------
    //  Native methods
    //--------------------------------------------------------------------------------------------
    private static native boolean nativeHasFormData();

    private static native void nativeClearFormData();
}
