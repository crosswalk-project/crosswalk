// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension;

import android.content.Context;
import android.content.Intent;

import org.chromium.base.CalledByNative;

import org.xwalk.core.extensions.XWalkExtensionAndroid;
import org.xwalk.runtime.extension.XWalkExtension;

/**
 * The extension bridge for the implementation based on xwalk core.
 */
class XWalkCoreExtensionBridge extends XWalkExtensionAndroid implements XWalkExtensionBridge {
    private XWalkExtension mExtension;

    public XWalkCoreExtensionBridge(XWalkExtension extension) {
        super(extension.getExtensionName(), extension.getJsApi());
        mExtension = extension;
    }

    //------------------------------------------------
    // XWalkExtensionBridge implementations.
    //------------------------------------------------
    public void handleMessage(int instanceID, String message) {
        mExtension.onMessage(instanceID, message);
    }

    public String handleSyncMessage(int instanceID, String message) {
        return mExtension.onSyncMessage(instanceID, message);
    }

    public void onDestroy() {
        mExtension.onDestroy();
        destroyExtension();
    }

    public void onResume() {
        mExtension.onResume();
    }

    public void onPause() {
        mExtension.onPause();
    }

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        mExtension.onActivityResult(requestCode, resultCode, data);
    }

    //------------------------------------------------
    // Overriden methods from XWalkExtensionAndroid
    //------------------------------------------------
    @Override
    public void postMessage(int instanceId, String message) {
        super.postMessage(instanceId, message);
    }

    @Override
    public void broadcastMessage(String message) {
        super.broadcastMessage(message);
    }

}
