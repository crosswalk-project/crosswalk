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

xwalk._setupExtensionInternal = function(extension_obj) {
  var callback_listeners = [];
  var callback_id = 0;

  function wrapCallback(args, callback) {
    if (callback) {
      var id = (callback_id++).toString();
      callback_listeners[id] = callback;
      args.unshift(id);
    } else {
      // The function name and the callback ID are prepended before
      // the arguments. If there is no callback, an empty string is
      // should be used. This will be sorted out by the InternalContext
      // message handler.
      args.unshift("");
    }
  }

  extension_obj.setMessageListener(function(msg) {
    var args = arguments[0];
    var id = args.shift();
    var listener = callback_listeners[id];

    if (listener !== undefined)
      listener.apply(null, args);
  });

  // All Internal Extensions functions should only be exposed by
  // this _internal object, acting like a namespace.
  extension_obj._internal = {};

  extension_obj._internal.postMessage = function(function_name, args, callback) {
    wrapCallback(args, callback);
    args.unshift(function_name);
    extension_obj.postMessage(args);
  };
}
