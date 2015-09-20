// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.xwview.test;

import org.xwalk.core.internal.extensions.XWalkExtensionAndroid;

public class ExtensionEchoInternal extends XWalkExtensionAndroid {

    public ExtensionEchoInternal() {
        super("echoJava",
              "var echoListener = null;"
              + "extension.setMessageListener(function(msg) {"
              + "  if (echoListener instanceof Function) {"
              + "    echoListener(msg);"
              + "  };"
              + "});"
              + "exports.echo = function(msg, callback) {"
              + "  echoListener = callback;"
              + "  extension.postMessage(msg);"
              + "};"
              + "exports.echoSync = function(msg) {"
              + "  return extension.internal.sendSyncMessage(msg);"
              + "};"
             );
    }

    public void onMessage(int instanceID, String message) {
        postMessage(instanceID, "From java:" + message);
    }

    public String onSyncMessage(int instanceID, String message) {
        return "From java sync:" + message;
    }

    public void onBinaryMessage(int instanceId, byte[] message) {
        postBinaryMessage(instanceId, message);
    }

    public void onDestroy() {
    }
}
