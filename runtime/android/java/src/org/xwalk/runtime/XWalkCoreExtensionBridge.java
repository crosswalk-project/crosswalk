// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime;

import android.content.Context;
import android.content.Intent;

import org.chromium.base.CalledByNative;
import org.xwalk.core.extensions.XWalkExtensionAndroid;
import org.xwalk.runtime.extension.XWalkExtension;

/**
 * This class is to bridge the extension system provided from runtime core
 * to our runtime extension system.
 */
class XWalkCoreExtensionBridge extends XWalkExtensionAndroid {
    private XWalkExtension mExtension;
    private XWalkRuntimeViewProvider mProvider;

    public XWalkCoreExtensionBridge(XWalkExtension extension, XWalkRuntimeViewProvider provider) {
        super(extension.getExtensionName(), extension.getJsApi());
        mExtension = extension;
        mProvider = provider;
    }

    @Override
    public void handleMessage(int instanceID, String message) {
        mProvider.onMessage(mExtension, instanceID, message);
    }

    @Override
    public String handleSyncMessage(int instanceID, String message) {
        return mProvider.onSyncMessage(mExtension, instanceID, message);
    }

    @Override
    @CalledByNative
    public void onDestroy() {
        mProvider.getExtensionContext().unregisterExtension(mExtension);
    }
}
