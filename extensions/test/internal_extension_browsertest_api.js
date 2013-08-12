// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

extension._setupExtensionInternal();

exports.Person = function(name, age) {
  this.name = name;
  this.age = age;
};

exports.clearDatabase = function() {
  extension._postMessageInternal('clearDatabase', []);
};

exports.addPerson = function(arg1, arg2) {
  extension._postMessageInternal('addPerson', [arg1, arg2]);
};

exports.addPersonObject = function(arg1) {
  extension._postMessageInternal('addPersonObject', [arg1]);
};

exports.getAllPersons = function(arg1, callback) {
  extension._setMessageListenerInternal('getAllPersons', [arg1], callback);
};

exports.getPersonAge = function(arg1, callback) {
  extension._setMessageListenerInternal('getPersonAge', [arg1], callback);
};
