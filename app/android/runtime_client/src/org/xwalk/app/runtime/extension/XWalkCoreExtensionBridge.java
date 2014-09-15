// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app.runtime.extension;

import android.content.Context;
import android.content.Intent;

import org.xwalk.core.XWalkExtension;

/**
 * The extension bridge for the implementation based on xwalk core.
 */
class XWalkCoreExtensionBridge extends XWalkExtension implements XWalkRuntimeExtensionBridge {
    private XWalkExtensionClient mExtension;

    public XWalkCoreExtensionBridge(XWalkExtensionClient extension) {
        super(extension.getExtensionName(), extension.getJsApi(), null);
        mExtension = extension;
    }

    //------------------------------------------------
    // XWalkExtensionBridge implementations.
    //------------------------------------------------
    public void onMessage(int instanceID, String message) {
        mExtension.onMessage(instanceID, message);
    }

    public String onSyncMessage(int instanceID, String message) {
        return mExtension.onSyncMessage(instanceID, message);
    }

    public void onDestroy() {
        mExtension.onDestroy();
    }

    public void onResume() {
        mExtension.onResume();
    }

    public void onPause() {
        mExtension.onPause();
    }

    public void onStart() {
        mExtension.onStart();
    }

    public void onStop() {
        mExtension.onStop();
    }

    public void onNewIntent(Intent intent) {
        mExtension.onNewIntent(intent);
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
