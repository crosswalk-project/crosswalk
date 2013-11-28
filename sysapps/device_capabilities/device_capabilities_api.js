// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var _promises = {};
var _next_promise_id = 0;
var _listeners = {};
var _next_listener_id = 0;

var Promise = requireNative('sysapps_promise').Promise;

var postMessage = function(msg) {
  var p = new Promise();

  _promises[_next_promise_id] = p;
  msg._promise_id = _next_promise_id.toString();
  _next_promise_id += 1;

  extension.postMessage(JSON.stringify(msg));
  return p;
};

exports.getCPUInfo = function() {
  var msg = {
    'cmd': 'getCPUInfo'
  };
  return postMessage(msg);
};

exports.getDisplayInfo = function() {
  var msg = {
    'cmd': 'getDisplayInfo'
  };
  return postMessage(msg);
};

exports.getMemoryInfo = function() {
  var msg = {
    'cmd': 'getMemoryInfo'
  };
  return postMessage(msg);
};

exports.getStorageInfo = function() {
  var msg = {
    'cmd': 'getStorageInfo'
  };
  return postMessage(msg);
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
    if (Array.isArray(obj[key])) {
      var obj_array = obj[key];
      var const_obj_array = [];
      for (var i = 0; i < obj_array.length; ++i) {
        var const_sub_obj = {};
        for (var sub_key in obj_array[i]) {
          _addConstProperty(const_sub_obj, sub_key, obj_array[i][sub_key]);
        }
        const_obj_array.push(const_sub_obj);
      }
      _addConstProperty(const_obj, key, const_obj_array);
    } else {
      _addConstProperty(const_obj, key, obj[key]);
    }
  }
  return const_obj;
}

extension.setMessageListener(function(json) {
  var msg = JSON.parse(json);

  if (msg.reply == 'attachStorage' ||
      msg.reply == 'detachStorage' ||
      msg.reply == 'connectDisplay' ||
      msg.reply == 'disconnectDisplay') {
    for (var id in _listeners) {
      if (_listeners[id]['eventName'] === msg.eventName) {
        _listeners[id]['callback'](_createConstClone(msg.data));
      }
    }
    return;
  }

  if (msg.data.error) {
    _promises[msg._promise_id].reject(msg.data.error);
  } else {
    _promises[msg._promise_id].fulfill(_createConstClone(msg.data)); 
  }

  delete _promises[msg._promise_id];
});

var _hasListener = function(eventName) {
  var count = 0;
  for (var i in _listeners) {
    if (_listeners[i]['eventName'] === eventName) {
      count += 1;
    }
  }
  return (0 !== count);
};

exports.addEventListener = function(eventName, callback) {
  if (typeof eventName !== 'string') {
    console.log("Invalid parameters (*, -)!");
    return -1;
  }

  if (typeof callback !== 'function') {
    console.log("Invalid parameters (-, *)!");
    return -1;
  }

  if (!_hasListener(eventName)) {
    var msg = {
      'cmd': 'addEventListener',
      'eventName': eventName
    };
    extension.postMessage(JSON.stringify(msg));
  }

  var listener = {
    'eventName': eventName,
    'callback': callback
  };

  var listener_id = _next_listener_id;
  _next_listener_id += 1;
  _listeners[listener_id] = listener;

  return listener_id;
};

var _sendSyncMessage = function(msg) {
  return extension.internal.sendSyncMessage(JSON.stringify(msg));
};
