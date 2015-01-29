// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var g_next_async_call_id = 0;
var g_async_calls = [];
var g_listeners = [];

// Preserve 4 spaces to hold onattach, ondetach, onconnect and ondisconnect's
// callback functions.
var g_next_listener_id = 4;

function AsyncCall(resolve, reject) {
  this.resolve = resolve;
  this.reject = reject;
}

function createPromise(msg) {
  var promise = new Promise(function(resolve, reject) {
    g_async_calls[g_next_async_call_id] = new AsyncCall(resolve, reject);
  });
  msg.asyncCallId = g_next_async_call_id;
  extension.postMessage(JSON.stringify(msg));
  ++g_next_async_call_id;
  return promise;
}

exports.getCPUInfo = function() {
  var msg = {
    'cmd': 'getCPUInfo'
  };
  return createPromise(msg);
};

exports.getAVCodecs = function() {
  var msg = {
    'cmd': 'getCodecsInfo'
  };
  return createPromise(msg);
};

exports.getDisplayInfo = function() {
  var msg = {
    'cmd': 'getDisplayInfo'
  };
  return createPromise(msg);
};

exports.getMemoryInfo = function() {
  var msg = {
    'cmd': 'getMemoryInfo'
  };
  return createPromise(msg);
};

exports.getStorageInfo = function() {
  var msg = {
    'cmd': 'getStorageInfo'
  };
  return createPromise(msg);
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
    for (var id in g_listeners) {
      if (g_listeners[id]['eventName'] === msg.eventName) {
        var event = null;
        if (msg.eventName == 'displayconnect' ||
            msg.eventName == 'displaydisconnect') {
          event = new SystemDisplayEvent(msg.data);
        }
        if (msg.eventName == 'storageattach' ||
            msg.eventName == 'storagedetach') {
          event = new SystemStorageEvent(msg.data);
        }
        g_listeners[id]['callback'](event);
      }
    }
    return;
  }

  if (msg.data.error) {
    g_async_calls[msg.asyncCallId].reject(msg.data.error);
  } else {
    g_async_calls[msg.asyncCallId].resolve(_createConstClone(msg.data)); 
  }

  delete g_async_calls[msg.asyncCallId];
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
        g_listeners[0] = listener;
        listener_id = 0;
        break;

      case 'storagedetach':
        g_listeners[1] = listener;
        listener_id = 1;
        break;

      case 'displayconnect':
        g_listeners[2] = listener;
        listener_id = 2;
        break;

      case 'displaydisconnect':
        g_listeners[3] = listener;
        listener_id = 3;
        break;

      default:
        console.log("Invalid event name!");
        break;
    }
  } else {
      listener_id = g_next_listener_id;
      g_next_listener_id += 1;
      g_listeners[listener_id] = listener;
  }

  if (g_listeners[listener_id] != null) {
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
};

var _sendSyncMessage = function(msg) {
  return extension.internal.sendSyncMessage(JSON.stringify(msg));
};

window.SystemDisplayEvent = function(data) {
  _addConstProperty(this, 'display', _createConstClone(data));
  this.prototype = new Event('SystemDisplayEvent');
};

window.SystemStorageEvent = function(data) {
  _addConstProperty(this, 'storage', _createConstClone(data));
  this.prototype = new Event('SystemStorageEvent');
};
