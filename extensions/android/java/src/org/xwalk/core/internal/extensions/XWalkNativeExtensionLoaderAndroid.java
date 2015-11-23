// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.extensions;

import org.chromium.base.annotations.JNINamespace;

@JNINamespace("xwalk::extensions")
public abstract class XWalkNativeExtensionLoaderAndroid {
    public void registerNativeExtensionsInPath(String path) {
        nativeRegisterExtensionInPath(path);
    }

    private static native void nativeRegisterExtensionInPath(String path);
}
