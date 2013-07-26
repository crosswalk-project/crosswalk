// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var xwalk = xwalk || {};

xwalk.postMessage = function(extension, msg) {
  native function PostMessage();
  PostMessage(extension, msg);
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
  native function SendSyncMessage();
  return SendSyncMessage(extension, msg);
}
