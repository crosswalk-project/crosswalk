// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var _promises = {};
var _next_promise_id = 0;
var _listeners = {};
// Preserve 4 spaces to hold onattach, ondetach, onconnect and ondisconnect's
// callback functions.
var _next_listener_id = 4;

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

exports.getAVCodecs = function() {
  var msg = {
    'cmd': 'getCodecsInfo'
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

function _addEventListener(isOn, eventName, callback) {
  if (typeof eventName !== 'string') {
    console.log("Invalid parameters of eventName!");
    return -1;
  }

  if (!isOn && (typeof callback !== 'function')) {
    console.log("Invalid parameters of callback!");
    return -1;
  }

  if (isOn && (typeof callback !== null) && (typeof callback !== 'function')) {
    console.log("Invalid parameters of callback!");
    return -1;
  }

  var listener = {
    'eventName': eventName,
    'callback': callback
  };

  var listener_id;

  if (isOn) {
    switch(listener.eventName) {
      case 'storageattach':
        _listeners[0] = listener;
        listener_id = 0;
        break;

      case 'storagedetach':
        _listeners[1] = listener;
        listener_id = 1;
        break;

      case 'displayconnect':
        _listeners[2] = listener;
        listener_id = 2;
        break;

      case 'displaydisconnect':
        _listeners[3] = listener;
        listener_id = 3;
        break;

      default:
        console.log("Invalid event name!");
        break;
    }
  } else {
      listener_id = _next_listener_id;
      _next_listener_id += 1;
      _listeners[listener_id] = listener;
  }

  if (_listeners[listener_id] != null) {
    var msg = {
      'cmd': 'addEventListener',
      'eventName': listener.eventName
    };
    extension.postMessage(JSON.stringify(msg));
  }

  return listener_id;
}

Object.defineProperty(exports, 'onstorageattach', {
  set: function(callback) {
    _addEventListener(true, 'storageattach', callback);
  }
});

Object.defineProperty(exports, 'onstoragedetach', {
  set: function(callback) {
    _addEventListener(true, 'storagedetach', callback);
  }
});

Object.defineProperty(exports, 'ondisplayconnect', {
  set: function(callback) {
    _addEventListener(true, 'displayconnect', callback);
  }
});

Object.defineProperty(exports, 'ondisplaydisconnect', {
  set: function(callback) {
    _addEventListener(true, 'displaydisconnect', callback);
  }
});

exports.addEventListener = function(eventName, callback) {
  return _addEventListener(false, eventName, callback);
}

var _sendSyncMessage = function(msg) {
  return extension.internal.sendSyncMessage(JSON.stringify(msg));
};
