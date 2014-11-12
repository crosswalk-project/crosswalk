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
    private String mSocketName = null;

    // Defines what processes may access to the socket.
    public enum Security {
        // Use content::CanUserConnectToDevTools to authorize access to the socket.
        DEFAULT,

        // In addition to default authorization allows access to an app with android permission
        // named chromeAppPackageName + DEBUG_PERMISSION_SUFFIX.
        ALLOW_DEBUG_PERMISSION,

        // Allow other apps to access the web socket url to remote debug.
        ALLOW_SOCKET_ACCESS,
    }

    public XWalkDevToolsServer(String socketName) {
        mNativeDevToolsServer = nativeInitRemoteDebugging(socketName);
        mSocketName = socketName;
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
        boolean allowSocketAccess = security == Security.ALLOW_SOCKET_ACCESS;
        nativeSetRemoteDebuggingEnabled(
                mNativeDevToolsServer, enabled, allowDebugPermission, allowSocketAccess);
    }

    public void setRemoteDebuggingEnabled(boolean enabled) {
        setRemoteDebuggingEnabled(enabled, Security.DEFAULT);
    }

    public String getSocketName() {
        return mSocketName;
    }

    private native long nativeInitRemoteDebugging(String socketName);
    private native void nativeDestroyRemoteDebugging(long devToolsServer);
    private native boolean nativeIsRemoteDebuggingEnabled(long devToolsServer);
    private native void nativeSetRemoteDebuggingEnabled(
            long devToolsServer, boolean enabled, boolean allowDebugPermission, boolean allowSocketAccess);

    @CalledByNative
    private static boolean checkDebugPermission(Context context, int pid, int uid) {
        String debugPermissionName = context.getPackageName() + DEBUG_PERMISSION_SIFFIX;
        return context.checkPermission(debugPermissionName, pid, uid)
                == PackageManager.PERMISSION_GRANTED;
    }
}
