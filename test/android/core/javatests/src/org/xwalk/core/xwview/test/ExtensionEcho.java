// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import org.xwalk.core.extensions.XWalkExtensionAndroid;

public class ExtensionEcho extends XWalkExtensionAndroid {

    public ExtensionEcho() {
        super("echo",
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

    public void handleMessage(String message) {
        postMessage("From java:" + message);
    }

    public String handleSyncMessage(String message) {
        return "From java:" + message;
    }

    public void onDestroy() {
    }
}
