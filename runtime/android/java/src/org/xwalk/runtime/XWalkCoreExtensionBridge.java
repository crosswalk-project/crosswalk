// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime;

import android.content.Context;
import android.content.Intent;

// TODO(yongsheng): Enable it once it's done.
// import org.xwalk.core.extensions.XWalkExtensionBridge;
import org.xwalk.runtime.extension.XWalkExtension;

/**
 * This class is to bridge the extension system provided from runtime core
 * to our runtime extension system.
 */
class XWalkCoreExtensionBridge /* extends XWalkExtensionBridge */ {
    private XWalkExtension mExtension;
    private XWalkRuntimeViewProvider mProvider;

    public XWalkCoreExtensionBridge(XWalkExtension extension, XWalkRuntimeViewProvider provider) {
        // super(extension.getApiVersion(), extension.getExtensionName(), extension.getJsApi());
        mExtension = extension;
        mProvider = provider;
    }

    public void handleMessage(String message) {
        mProvider.onMessage(mExtension, message);
    }

    public void handleSyncMessage(String message) {
        mProvider.onSyncMessage(mExtension, message);
    }

    // TODO(yongsheng): Depends on runtime core impl.
    public void postMessage(String message) {
    }
}
