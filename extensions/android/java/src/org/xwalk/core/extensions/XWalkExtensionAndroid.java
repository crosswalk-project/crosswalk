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
    private ArrayList<Integer> mInstances;

    public XWalkExtensionAndroid(String name, String jsApi) {
        mXWalkExtension = nativeCreateExtension(name, jsApi);
        mInstances = new ArrayList<Integer>();
    }

    public void destroy() {
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
        for(Integer i : mInstances) {
            postMessage(i, message);
        }
    }

    @CalledByNative
    public abstract void handleMessage(int instanceID, String message);

    @CalledByNative
    public abstract String handleSyncMessage(int instanceID, String message);

    @CalledByNative
    public abstract void onDestroy();

    @CalledByNative
    private void onInstanceCreated(int instanceID) {
        if(!mInstances.contains(new Integer(instanceID)))
            mInstances.add(new Integer(instanceID));
    }

    @CalledByNative
    private void onInstanceRemoved(int instanceID) {
        mInstances.remove(new Integer(instanceID));
    }

    private native int nativeCreateExtension(String name, String jsApi);
    private native void nativePostMessage(int nativeXWalkExtensionAndroid, int instanceID, String message);
    private native void nativeDestroyExtension(int nativeXWalkExtensionAndroid);
}
