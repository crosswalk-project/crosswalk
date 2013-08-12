// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  return function (requireNative) {
    var postMessage = requireNative('PostMessage');
    var sendSyncMessage = requireNative('SendSyncMessage');
    var xwalk = {};

    xwalk.postMessage = function(extension, msg) {
      //native function PostMessage();
      postMessage(extension, msg);
    };

    xwalk._message_listeners = {};

    xwalk.setMessageListener = function(extension, callback) {
      if (callback === undefined)
        delete xwalk._message_listeners[extension];
      else
        xwalk._message_listeners[extension] = callback;
    };

    xwalk.onpostmessage = function(extension, msg) {
      var listener = xwalk._message_listeners[extension];
      if (listener !== undefined)
        listener(msg);
    };

    xwalk.sendSyncMessage = function(extension, msg) {
      //native function SendSyncMessage();
      return sendSyncMessage(extension, msg);
    };

    return xwalk;
  }
})()
