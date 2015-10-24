// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.extensions;

import android.util.Log;

import java.util.ArrayList;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;

/**
 * This class is the implementation class for XWalkExtension by calling internal
 * XWalkExtension class.
 */
@JNINamespace("xwalk::extensions")
public abstract class XWalkExtensionAndroid {
    private final static String TAG = "XWalkExtensionAndroid";
    private long mXWalkExtension;

    public XWalkExtensionAndroid(String name, String jsApi) {
        mXWalkExtension = nativeGetOrCreateExtension(name, jsApi, null);
    }

    public XWalkExtensionAndroid(String name, String jsApi, String[] entryPoints) {
        mXWalkExtension = nativeGetOrCreateExtension(name, jsApi, entryPoints);
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

    public void postBinaryMessage(int instanceID, byte[] message) {
        if (mXWalkExtension == 0) {
            Log.e(TAG, "Can not post a binary message to an invalid extension!");
            return;
        }

        nativePostBinaryMessage(mXWalkExtension, instanceID, message);
    }

    public void broadcastMessage(String message) {
        if (mXWalkExtension == 0) {
            Log.e(TAG, "Can not broadcast message to an invalid extension!");
            return;
        }

        nativeBroadcastMessage(mXWalkExtension, message);
    }

    @CalledByNative
    public void onInstanceCreated(int instanceID) {}

    @CalledByNative
    public void onInstanceDestroyed(int instanceID) {}

    @CalledByNative
    public abstract void onMessage(int instanceID, String message);

    @CalledByNative
    public void onBinaryMessage(int instanceID, byte[] message) {}

    @CalledByNative
    public abstract String onSyncMessage(int instanceID, String message);

    private native long nativeGetOrCreateExtension(String name, String jsApi, String[] entryPoints);
    private native void nativePostMessage(long nativeXWalkExtensionAndroid, int instanceID, String message);
    private native void nativePostBinaryMessage(long nativeXWalkExtensionAndroid, int instanceID, byte[] message);
    private native void nativeBroadcastMessage(long nativeXWalkExtensionAndroid, String message);
    private native void nativeDestroyExtension(long nativeXWalkExtensionAndroid);
}
