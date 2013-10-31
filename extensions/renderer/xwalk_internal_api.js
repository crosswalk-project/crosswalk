// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var callback_listeners = {};
var callback_id = 0;
var extension_object;

function wrapCallback(args, callback) {
  if (callback) {
    var id = (callback_id++).toString();
    callback_listeners[id] = callback;
    args.unshift(id);
  } else {
    // The function name and the callback ID are prepended before
    // the arguments. If there is no callback, an empty string is
    // should be used. This will be sorted out by the InternalInstance
    // message handler.
    args.unshift("");
  }

  return id;
}

exports.setupInternalExtension = function(extension_obj) {
  if (extension_object != null)
    return;

  extension_object = extension_obj;

  extension_object.setMessageListener(function(msg) {
    var args = arguments[0];
    var id = args.shift();
    var listener = callback_listeners[id];

    if (listener !== undefined) {
      if (!listener.apply(null, args))
        delete callback_listeners[id];
    }
  });
};

exports.postMessage = function(function_name, args, callback) {
  var id = wrapCallback(args, callback);
  args.unshift(function_name);
  extension_object.postMessage(args);

  return id;
};

exports.removeCallback = function(id) {
  if (!id in callback_listeners)
    return;

  delete callback_listeners[id];
};
