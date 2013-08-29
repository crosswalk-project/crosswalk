// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extensions;

import org.chromium.base.CalledByNative;
import org.chromium.base.JNINamespace;

/**
 * This class is the implementation class for XWalkExtension by calling internal
 * XWalkExtension class.
 */
@JNINamespace("xwalk::extensions")
public abstract class XWalkExtensionAndroid {
    private int mXWalkExtension;

    public XWalkExtensionAndroid(String name, String jsApi) {
        mXWalkExtension = nativeCreateExtension(name, jsApi);
    }

    public void postMessage(String message) {
        nativePostMessage(mXWalkExtension, message);
    }

    @CalledByNative
    public abstract void handleMessage(String message);

    @CalledByNative
    public abstract String handleSyncMessage(String message);

    @CalledByNative
    public abstract void onDestroy();

    private native int nativeCreateExtension(String name, String jsApi);
    private native void nativePostMessage(int nativeXWalkExtensionAndroid, String message);
}
