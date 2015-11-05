// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var g_next_async_call_id = 0;
var g_async_calls = [];
var g_listeners = [];

var WIFI_STATE_CHANGED_EVENT = 'wifistatechanged';
var PEERS_CHANGED_EVENT = 'peerschanged';
var CONNECTION_CHANGED_EVENT = 'connectionchanged';
var THIS_DEVICE_CHANGED_EVENT = 'thisdevicechanged';
var DISCOVERY_STOPPED_EVENT = 'discoverystoppedevent';

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

exports.init = function() {
  var msg = {
    'cmd': 'init'
  };
  return createPromise(msg);
};

exports.discoverPeers = function() {
  var msg = {
    'cmd': 'discoverPeers'
  };
  return createPromise(msg);
};

exports.stopDiscovery = function() {
  var msg = {
    'cmd': 'stopPeerDiscovery'
  };
  return createPromise(msg);
};

exports.getPeers = function() {
  var msg = {
    'cmd': 'getPeers'
  };
  return createPromise(msg);
};

exports.getConnectionInfo = function() {
  var msg = {
    'cmd': 'getConnectionInfo'
  };
  return createPromise(msg);
};

exports.ConnectionMethod = {
  PBC : "pbc",
  Display : "display",
  Label : "label",
  Keyboard : "keyboard"
};

exports.connect = function(device) {
  var msg = {
    'cmd': 'connect',
    'data': device,
    'method' : "pbc"
  };
  return createPromise(msg);
};

exports.cancelConnect = function() {
  var msg = {
    'cmd': 'cancelConnect'
  };
  return createPromise(msg);
};

exports.disconnect = function() {
  var msg = {
    'cmd': 'disconnect'
  };
  return createPromise(msg);
};

function Device(data) {
  this.MAC = data.MAC;
  this.name = data.name;
  this.type = data.type;
  this.status = data.status;
}

function _addEventListener(isOn, eventName, callback) {
  if (typeof eventName !== 'string') {
    console.warn("Invalid parameters of eventName!");
    return -1;
  }

  if (!isOn && (typeof callback !== 'function')) {
    console.warn("Invalid parameters of callback!");
    return -1;
  }

  if (isOn && (typeof callback !== null) && (typeof callback !== 'function')) {
    console.warn("Invalid parameters of callback!");
    return -1;
  }

  var listener = {
    'eventName': eventName,
    'callback': callback
  };

  var listener_id;

  if (isOn) {
    switch(listener.eventName) {
    case PEERS_CHANGED_EVENT:
      g_listeners[0] = listener;
      listener_id = 0;
      break;
    case PEERS_CHANGED_EVENT:
      g_listeners[1] = listener;
      listener_id = 1;
      break;
    case CONNECTION_CHANGED_EVENT:
      g_listeners[2] = listener;
      listener_id = 2;
      break;
    case THIS_DEVICE_CHANGED_EVENT:
      g_listeners[3] = listener;
      listener_id = 3;
      break;
    case DISCOVERY_STOPPED_EVENT:
      g_listeners[4] = listener;
      listener_id = 4;
      break;
    default:
      console.warn("Invalid event name!");
    break;
    }
  } else {
    listener_id = g_next_async_call_id;
    g_next_async_call_id += 1;
    g_listeners[listener_id] = listener;
  }
  return listener_id;
}

Object.defineProperty(exports, 'onwifistatechanged', {
  set: function(callback) {
    _addEventListener(true, WIFI_STATE_CHANGED_EVENT, callback);
  }
});

Object.defineProperty(exports, 'onpeerschanged', {
  set: function(callback) {
    _addEventListener(true, PEERS_CHANGED_EVENT, callback);
  }
});

Object.defineProperty(exports, 'onconnectionchanged', {
  set: function(callback) {
    _addEventListener(true, CONNECTION_CHANGED_EVENT, callback);
  }
});

Object.defineProperty(exports, 'onthisdevicechanged', {
  set: function(callback) {
    _addEventListener(true, THIS_DEVICE_CHANGED_EVENT, callback);
  }
});

Object.defineProperty(exports, 'ondiscoverystopped', {
  set: function(callback) {
    _addEventListener(true, DISCOVERY_STOPPED_EVENT, callback);
  }
});

exports.addEventListener = function(eventName, callback) {
  return _addEventListener(false, eventName, callback);
}

function handleEvent(msg) {
  for (var id in g_listeners) {
    if (g_listeners[id]['eventName'] === msg.eventName && g_listeners[id]['callback']) {
      g_listeners[id]['callback'](msg.data);
    }
  }
}

function handlePromise(msgObj) {
  if (msgObj.data.error) {
    g_async_calls[msgObj.asyncCallId].reject(msgObj.data.error);
  } else {
    g_async_calls[msgObj.asyncCallId].resolve(msgObj.data);
  }

  delete g_async_calls[msgObj.asyncCallId];
}

extension.setMessageListener(function(json) {
  var _msg = JSON.parse(json);
  switch (_msg.eventName) {
    case WIFI_STATE_CHANGED_EVENT:
    case PEERS_CHANGED_EVENT:
    case CONNECTION_CHANGED_EVENT:
    case THIS_DEVICE_CHANGED_EVENT:
    case DISCOVERY_STOPPED_EVENT:
    {
      handleEvent(_msg);
      return;
    }
    default:
      break;
  }
  handlePromise(_msg);
});