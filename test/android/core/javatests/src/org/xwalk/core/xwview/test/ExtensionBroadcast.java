// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import org.xwalk.core.extensions.XWalkExtensionAndroid;

public class ExtensionBroadcast extends XWalkExtensionAndroid {

    public ExtensionBroadcast() {
        super("broadcast",
              "exports.setHandler = function(handler) {"
              + "  extension.setMessageListener(handler);"
              + "};"
              + "exports.trigger = function(msg) {"
              + "  extension.postMessage(msg);"
              + "};"
             );
    }

    public void handleMessage(int instanceID, String message) {
        broadcastMessage("From java broadcast:" + message);
    }

    public String handleSyncMessage(int instanceID, String message) {
        return "From java:" + message;
    }

    public void onDestroy() {
    }
}
