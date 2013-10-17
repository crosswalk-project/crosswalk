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
    private int mXWalkExtensionInstanceID = 0;

    public XWalkExtensionAndroid(String name, String jsApi) {
        mXWalkExtension = nativeCreateExtension(name, jsApi);
    }

    public void postMessage(String message) {
        if (mXWalkExtensionInstanceID != 0) {
            nativePostMessage(mXWalkExtension, mXWalkExtensionInstanceID, message);
        }
    }

    @CalledByNative
    public abstract void handleMessage(String message);

    @CalledByNative
    public abstract String handleSyncMessage(String message);

    @CalledByNative
    public abstract void onDestroy();

    /* FIXME(halton): Internal WebFrame is not exposed in Java side. With that
     * fact, if multiple instances alive(iframe), there is no way to identify
     * which instance to send message. We only keep the first instance id
     * in Java. Thus all extension instances created on native side can send
     * messages to Java side, but Java side can only send messages to the first
     * instance.
     */
    @CalledByNative
    private void onInstanceCreated(int instanceID) {
        if (mXWalkExtensionInstanceID != 0)
            mXWalkExtensionInstanceID = instanceID;
    }

    @CalledByNative
    private void onInstanceRemoved() {
        mXWalkExtensionInstanceID = 0;
    }

    private native int nativeCreateExtension(String name, String jsApi);
    private native void nativePostMessage(int nativeXWalkExtensionAndroid, int instanceID, String message);
}
