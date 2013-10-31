// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var internal = requireNative("internal");
internal.setupInternalExtension(extension);

var heartbeatCallbackId;

exports.Person = function(name, age) {
  this.name = name;
  this.age = age;
};

exports.clearDatabase = function() {
  internal.postMessage('clearDatabase', []);
};

exports.addPerson = function(arg1, arg2) {
  internal.postMessage('addPerson', [arg1, arg2]);
};

exports.addPersonObject = function(arg1) {
  internal.postMessage('addPersonObject', [arg1]);
};

exports.getAllPersons = function(arg1, callback) {
  internal.postMessage('getAllPersons', [arg1], callback);
};

exports.getPersonAge = function(arg1, callback) {
  internal.postMessage('getPersonAge', [arg1], callback);
};

exports.startHeartbeat = function(callback) {
  if (heartbeatCallbackId != null)
    return;

  heartbeatCallbackId = internal.postMessage('startHeartbeat', [], callback);
};

exports.stopHeartbeat = function() {
  if (heartbeatCallbackId == null)
    return;

  internal.removeCallback(heartbeatCallbackId);
  heartbeatCallbackId = null;

  internal.postMessage('stopHeartbeat', []);
};
