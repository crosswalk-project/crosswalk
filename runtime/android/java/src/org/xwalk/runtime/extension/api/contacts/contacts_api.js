// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var _promises = {};
var _next_promise_id = 0;
var _listeners = [];

// Preserve first element for "oncontactschange" way event listener.
//
// Compared with addEventListener, the main difference is there is only one
// oncontactschange callback will be invoked. While addEventListener can 
// add multiple callbacks when event "contactschanged" arrives.
//
// This listener can be set as "oncontactschange = function(){...};"
// and can be unset as "oncontactschange = null;"
_listeners[0] = null;
var _next_listener_id = 1;

var Promise = requireNative('sysapps_promise').Promise;

var postMessage = function(msg) {
  var p = new Promise();

  _promises[_next_promise_id] = p;
  msg._promise_id = _next_promise_id.toString();
  _next_promise_id += 1;

  extension.postMessage(JSON.stringify(msg));
  return p;
};

function _addConstProperty(obj, propertyKey, propertyValue) {
  Object.defineProperty(obj, propertyKey, {
    configurable: false,
    writable: false,
    value: propertyValue
  });
}

function _createConstClone(obj) {
  var const_obj = {};
  for (var key in obj) {
    _addConstProperty(const_obj, key, obj[key]);
  }
  return const_obj;
}

extension.setMessageListener(function(json) {
  var msg = JSON.parse(json);

  if (msg.reply == 'contactschange') {
    for (var id in _listeners) {
      if (typeof _listeners[id] === 'function') {
        _listeners[id](_createConstClone(msg.data));
      }
    }
    return;
  }

  if (msg.data.error) {
    _promises[msg._promise_id].reject(msg.data.error);
  } else {
    _promises[msg._promise_id].fulfill(msg.data);
  }

  delete _promises[msg._promise_id];
});

exports.save = function(contact) {
  var msg = {};
  msg['cmd'] = 'save';
  msg['contact'] = contact;
  return postMessage(msg);
};

exports.find = function(options) {
  var msg = {};
  msg['cmd'] = 'find';
  msg['options'] = options;
  return postMessage(msg);
};

exports.remove = function(contactId) {
  var msg = {};
  msg['cmd'] = 'remove';
  msg['contactId'] = contactId;
  return postMessage(msg);
};

function _addListener(isOnChange, callback) {
  // Check validation of callback for addEventListener way.
  if (!isOnChange && (typeof callback !== 'function')) {
    console.log('Invalid parameters of callback!');
    return -1;
  }
  
  // Check validation of callback for oncontactschanged way, it can be null or a function.
  if (isOnChange && (callback !== null) && (typeof callback !== 'function')) {
    console.log('Invalid parameters of callback!');
    return -1;
  }

  var listener_id;
  if (isOnChange) { // Set callback to oncontactschange()
      _listeners[0] = callback;
      listener_id = 0;
  } else { // Set callback by addEventListner()
      listener_id = _next_listener_id;
      _next_listener_id += 1;
      _listeners[listener_id] = callback;
  }

  // Notify native code there is valid listener.
  if (_listeners[0] != null || _listeners.length > 1) {
    var msg = { 'cmd': 'addEventListener' };
    extension.postMessage(JSON.stringify(msg));
  }

  return listener_id;
}

function _addEventListener(callback) {
  return _addListener(false, callback);
}

Object.defineProperty(exports, 'oncontactschange', {
  set: function(callback) {
    _addListener(true, callback);
  }
});

exports.addEventListener = _addEventListener;
