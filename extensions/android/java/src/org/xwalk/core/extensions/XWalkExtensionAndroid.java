// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extensions;

import java.util.ArrayList;
import org.chromium.base.CalledByNative;
import org.chromium.base.JNINamespace;

/**
 * This class is the implementation class for XWalkExtension by calling internal
 * XWalkExtension class.
 */
@JNINamespace("xwalk::extensions")
public abstract class XWalkExtensionAndroid {
    private int mXWalkExtension;
    private ArrayList<Integer> mInstances;

    public XWalkExtensionAndroid(String name, String jsApi) {
        mXWalkExtension = nativeCreateExtension(name, jsApi);
        mInstances = new ArrayList<Integer>();
    }

    public void postMessage(int instanceID, String message) {
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
}
