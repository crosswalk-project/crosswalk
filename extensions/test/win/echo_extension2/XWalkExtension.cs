// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

using System;

namespace xwalk
{
public class XWalkExtension
{
  public XWalkExtension() {
  }

  public String ExtensionName() {
    return "echo2";
  }

  public String ExtensionAPI() {
    return
      @"var echoListener = null;
        extension.setMessageListener(function(msg) {
          if (echoListener instanceof Function) {
            echoListener(msg);
          };
        });
        exports.echo2 = function(msg, callback) {
          echoListener = callback;
          extension.postMessage(msg);
        };
        exports.syncEcho2 = function(msg) {
          return extension.internal.sendSyncMessage(msg);
        };";
  }
}
}
