// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extensions;

import android.util.Log;

import java.util.ArrayList;

import org.chromium.base.CalledByNative;
import org.chromium.base.JNINamespace;

/**
 * This class is the implementation class for XWalkExtension by calling internal
 * XWalkExtension class.
 */
@JNINamespace("xwalk::extensions")
public abstract class XWalkExtensionAndroid {
    private final static String TAG = "XWalkExtensionAndroid";
    private int mXWalkExtension;

    public XWalkExtensionAndroid(String name, String jsApi) {
        mXWalkExtension = nativeCreateExtension(name, jsApi);
    }

    protected void destroyExtension() {
        if (mXWalkExtension == 0) {
            Log.e(TAG, "The extension to be destroyed is invalid!");
            return;
        }

        nativeDestroyExtension(mXWalkExtension);
        mXWalkExtension = 0;
    }

    public void postMessage(int instanceID, String message) {
        if (mXWalkExtension == 0) {
            Log.e(TAG, "Can not post a message to an invalid extension!");
            return;
        }

        nativePostMessage(mXWalkExtension, instanceID, message);
    }

    public void broadcastMessage(String message) {
        if (mXWalkExtension == 0) {
            Log.e(TAG, "Can not broadcast message to an invalid extension!");
            return;
        }

        nativeBroadcastMessage(mXWalkExtension, message);
    }

    @CalledByNative
    public abstract void handleMessage(int instanceID, String message);

    @CalledByNative
    public abstract String handleSyncMessage(int instanceID, String message);

    private native int nativeCreateExtension(String name, String jsApi);
    private native void nativePostMessage(int nativeXWalkExtensionAndroid, int instanceID, String message);
    private native void nativeBroadcastMessage(int nativeXWalkExtensionAndroid, String message);
    private native void nativeDestroyExtension(int nativeXWalkExtensionAndroid);
}
