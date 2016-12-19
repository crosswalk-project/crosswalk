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
    return "binaryTest";
  }

  public String ExtensionAPI() {
    return
      @"var binaryTestListener = null;
        extension.setMessageListener(function(msg) {
          if (binaryTestListener instanceof Function) {
            binaryTestListener(msg);
          };
        });
        exports.binaryEcho = function(msg, callback) {
          binaryTestListener = callback;
          extension.postMessage(msg);
        };";
  }
}
}
