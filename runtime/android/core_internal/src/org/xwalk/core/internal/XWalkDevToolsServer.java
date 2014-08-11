// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import org.chromium.base.JNINamespace;
import android.content.Context;
import android.content.pm.PackageManager;
import org.chromium.base.CalledByNative;

/**
 * Controller for Remote Web Debugging (Developer Tools).
 */
@JNINamespace("xwalk")
class XWalkDevToolsServer {
    private static final String DEBUG_PERMISSION_SIFFIX = ".permission.DEBUG";

    private long mNativeDevToolsServer = 0;

    // Defines what processes may access to the socket.
    public enum Security {
        // Use content::CanUserConnectToDevTools to authorize access to the socket.
        DEFAULT,

        // In addition to default authorization allows access to an app with android permission
        // named chromeAppPackageName + DEBUG_PERMISSION_SUFFIX.
        ALLOW_DEBUG_PERMISSION,
    }

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

    public void setRemoteDebuggingEnabled(boolean enabled, Security security) {
        boolean allowDebugPermission = security == Security.ALLOW_DEBUG_PERMISSION;
        nativeSetRemoteDebuggingEnabled(mNativeDevToolsServer, enabled, allowDebugPermission);
    }

    public void setRemoteDebuggingEnabled(boolean enabled) {
        setRemoteDebuggingEnabled(enabled, Security.DEFAULT);
    }

    public void allowConnectionFromUid(int uid) {
        nativeAllowConnectionFromUid(mNativeDevToolsServer, uid);
    }

    private native long nativeInitRemoteDebugging(String socketName);
    private native void nativeDestroyRemoteDebugging(long devToolsServer);
    private native boolean nativeIsRemoteDebuggingEnabled(long devToolsServer);
    private native void nativeSetRemoteDebuggingEnabled(
            long devToolsServer, boolean enabled, boolean allowDebugPermission);
    private native void nativeAllowConnectionFromUid(long devToolsServer, int uid);

    @CalledByNative
    private static boolean checkDebugPermission(Context context, int pid, int uid) {
        String debugPermissionName = context.getPackageName() + DEBUG_PERMISSION_SIFFIX;
        return context.checkPermission(debugPermissionName, pid, uid)
                == PackageManager.PERMISSION_GRANTED;
    }
}
