// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import org.xwalk.core.XWalkExtension;

public class ExtensionEcho extends XWalkExtension {

    public ExtensionEcho() {
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

    @Override
    public void onMessage(int instanceID, String message) {
        postMessage(instanceID, "From java:" + message);
    }

    @Override
    public String onSyncMessage(int instanceID, String message) {
        return "From java sync:" + message;
    }

    @Override
    public void onBinaryMessage(int instanceId, byte[] message) {
        postBinaryMessage(instanceId, message);
    }
}
