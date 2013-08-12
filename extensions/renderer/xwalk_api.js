// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var xwalk = xwalk || {};

xwalk.postMessage = function(extension, msg) {
  native function PostMessage();
  PostMessage(extension, msg);
};

xwalk._callback_id = 0;
xwalk._message_listeners = {};
xwalk._message_listeners_internal = {};

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

xwalk._listener_internal = function() {
  var args = arguments[0];
  var id = args.shift();
  var listener = xwalk._message_listeners_internal[id];

  if (listener !== undefined)
    listener.apply(null, args);
};

xwalk._setupExtensionInternal = function(extension) {
  xwalk.setMessageListener(extension, xwalk._listener_internal);
}

xwalk._postMessageInternal = function(extension, function_name, args) {
  // The function name and the callback ID are prepended before
  // the arguments. If there is no callback, an empty string is
  // should be used. This will be sorted out by the InternalContext
  // message handler.
  args.unshift("");
  args.unshift(function_name);

  native function PostMessage();
  PostMessage(extension, args);
};

xwalk._setMessageListenerInternal = function(extension, function_name,
                                             args, callback) {
  if (callback) {
    var id = (xwalk._callback_id++).toString();
    xwalk._message_listeners_internal[id] = callback;
    args.unshift(id);
  } else
    args.unshift("");

  args.unshift(function_name);

  native function PostMessage();
  PostMessage(extension, args);
};
