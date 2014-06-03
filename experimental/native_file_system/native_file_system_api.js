// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var _promises = {};
var _next_promise_id = 0;

var Promise = requireNative('sysapps_promise').Promise;
var IsolatedFileSystem = requireNative('isolated_file_system');

var postMessage = function(msg, success, error) {
  var p = new Promise();
  p.then(success, error);

  _promises[_next_promise_id] = p;
  msg._promise_id = _next_promise_id.toString();
  _next_promise_id += 1;

  extension.postMessage(msg);
};

function _isFunction(fn) {
  return !!fn && !fn.nodeName &&
      fn.constructor != String && fn.constructor != RegExp &&
      fn.constructor != Array && /function/i.test(fn + "");
}

var NativeFileSystem = function() {
};

var requestNativeFileSystem = function(path, success, error) {
  var msg = new Object();
  msg.data = new Object();
  msg.data.virtual_root = path;
  msg.cmd = "requestNativeFileSystem";
  function get_file_system_id_success(data) {
    var fs = IsolatedFileSystem.getIsolatedFileSystem(data.file_system_id);
    success(fs);
  }
  postMessage(msg, get_file_system_id_success, error);
}

var getDirectoryList = function() {
  extension.internal.sendSyncMessage("get");
}

NativeFileSystem.prototype = new Object();
NativeFileSystem.prototype.constructor = NativeFileSystem;
NativeFileSystem.prototype.requestNativeFileSystem = requestNativeFileSystem;
NativeFileSystem.prototype.getDirectoryList = getDirectoryList;

exports = new NativeFileSystem();

function handlePromise(msgObj) {
  if (msgObj.data.error) {
    if (_isFunction(_promises[msgObj._promise_id].reject)) {
      _promises[msgObj._promise_id].reject(msgObj.data);
    }
  } else {
    if (_isFunction(_promises[msgObj._promise_id].fulfill)) {
      _promises[msgObj._promise_id].fulfill(msgObj.data);
    }
  }
  delete _promises[msgObj._promise_id];
}

extension.setMessageListener(function(msgStr) {
  // TODO(shawngao5): This part of code should be refactored.
  // Follow DeviceCapability extension way to implement.
  var msgObj = JSON.parse(msgStr);
  switch (msgObj.cmd) {
    case "requestNativeFileSystem_ret":
      handlePromise(msgObj);
    default:
      break;
  }
});
