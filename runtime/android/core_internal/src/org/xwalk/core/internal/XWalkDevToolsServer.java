// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import org.chromium.base.JNINamespace;

/**
 * Controller for Remote Web Debugging (Developer Tools).
 */
@JNINamespace("xwalk")
class XWalkDevToolsServer {

    private long mNativeDevToolsServer = 0;

    public XWalkDevToolsServer(String socketName) {
        mNativeDevToolsServer = nativeInitRemoteDebugging(socketName);
    }

    public void destroy() {
        nativeDestroyRemoteDebugging(mNativeDevToolsServer);
        mNativeDevToolsServer = 0;
    }

    public boolean isRemoteDebuggingEnabled() {
        return nativeIsRemoteDebuggingEnabled(mNativeDevToolsServer);
    }

    public void setRemoteDebuggingEnabled(boolean enabled) {
        nativeSetRemoteDebuggingEnabled(mNativeDevToolsServer, enabled);
    }

    public void allowConnectionFromUid(int uid) {
        nativeAllowConnectionFromUid(mNativeDevToolsServer, uid);
    }

    private native long nativeInitRemoteDebugging(String socketName);
    private native void nativeDestroyRemoteDebugging(long devToolsServer);
    private native boolean nativeIsRemoteDebuggingEnabled(long devToolsServer);
    private native void nativeSetRemoteDebuggingEnabled(long devToolsServer, boolean enabled);
    private native void nativeAllowConnectionFromUid(long devToolsServer, int uid);
}
