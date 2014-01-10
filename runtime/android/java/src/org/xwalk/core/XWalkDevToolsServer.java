// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.util.Log;

import org.chromium.base.JNINamespace;

/**
 * Controller for Remote Web Debugging (Developer Tools).
 */
@JNINamespace("xwalk")
public class XWalkDevToolsServer {
    private final static String TAG = "XWalkDevToolsServer";

    // The singleton instance for devtools server in application wide which associates
    // with a constant socket name.
    private static XWalkDevToolsServer sInstance;
    private static String sSocketName;

    private int mNativeDevToolsServer = 0;

    /**
     * Returns the singleton instance of devtools server listened on the given socket name.
     * Note this method is not thread safe.
     */
    public static XWalkDevToolsServer getInstance(String socketName) {
        // Ignore the different socket name if the devtools server is already started with
        // another socket name.
        if (sSocketName != null && sSocketName != socketName)
            Log.w(TAG, "The devtools server is already started. " +
                       "Ignore the different socket name: " + socketName);

        if (sInstance == null) {
            sSocketName = socketName;
            sInstance = new XWalkDevToolsServer(socketName);
        }
        return sInstance;
    }

    private XWalkDevToolsServer(String socketName) {
        mNativeDevToolsServer = nativeInitRemoteDebugging(socketName);
    }

    @Override
    protected void finalize() {
        nativeDestroyRemoteDebugging(mNativeDevToolsServer);
        mNativeDevToolsServer = 0;
    }

    public boolean isRemoteDebuggingEnabled() {
        if (mNativeDevToolsServer == 0) return false;
        return nativeIsRemoteDebuggingEnabled(mNativeDevToolsServer);
    }

    public void setRemoteDebuggingEnabled(boolean enabled) {
        if (mNativeDevToolsServer == 0) return;
        nativeSetRemoteDebuggingEnabled(mNativeDevToolsServer, enabled);
    }

    private native int nativeInitRemoteDebugging(String socketName);
    private native void nativeDestroyRemoteDebugging(int devToolsServer);
    private native boolean nativeIsRemoteDebuggingEnabled(int devToolsServer);
    private native void nativeSetRemoteDebuggingEnabled(int devToolsServer, boolean enabled);
}
