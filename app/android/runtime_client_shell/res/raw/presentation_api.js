/**
 * Copyright (c) 2013 Intel Corporation. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

var v8toolsNative = requireNative("v8tools");

var CHANGE_EVENT = 'change';
var DEFAULT_SESSION_START_EVENT = 'defaultsessionstart';
var _listeners = {};
var _displayAvailable = false;
var _nextRequestId = 0;
var _sessionRequests = {};

function DOMError(msg) {
  this.name = msg;
}

function SessionRequest(id, successCallback, errorCallback) {
  this._requestId = id;
  this._successCallback = successCallback;
  this._errorCallback = errorCallback;
}

function Session(presentationId) {
  this.id = presentationId;

  this.__defineGetter__('state', function() {
    return _displayAvailable;
  });
  this.__defineSetter__('onmessage', function(callback) {
    if (callback) {
      addEventListener("messagetohost", callback);
    } else {
      removeEventListener("messagetohost", this.onmessage);
    }
  });
  this.send = function(data) {
    var message = { "cmd": "SendMessageToRemoteDisplay", "id": this.id, "data": data };
    extension.postMessage(JSON.stringify(message));
  }
}

exports.PresentationSession = function(presentationId) {
  this.id = presentationId;

  this.__defineGetter__('state', function() {
    return _displayAvailable;
  });
  this.__defineSetter__('onmessage', function(callback) {
    if (callback) {
      addEventListener("messagetoremote", callback);
    } else {
      removeEventListener("messagetoremote", this.onmessage);
    }
  });
  this.send = function(data) {
    var message = { "cmd": "SendMessageToHostDisplay", "id": this.id, "data": data };
    extension.postMessage(JSON.stringify(message));
  }
}

function Availability() {
  this.value = false;
  this.__defineSetter__("on" + CHANGE_EVENT, function(callback) {
    if (callback) {
      addEventListener(CHANGE_EVENT, callback);
    } else {
      removeEventListener(CHANGE_EVENT, this.onchange);
    }
  });
}

exports.__defineSetter__("on" + DEFAULT_SESSION_START_EVENT, function(callback) {
    if (callback) {
      addEventListener(DEFAULT_SESSION_START_EVENT, callback);
    } else {
      removeEventListener(DEFAULT_SESSION_START_EVENT, this.ondefaultsessionstart);
    }
});

exports.addEventListener = function(name, callback, useCapture /* ignored */) {
  if (typeof name !== "string" || typeof callback !== "function") {
    console.error("Invalid parameter for presentation.addEventListener!");
    return;
  }

  if (!_listeners[name])
    _listeners[name] = [];
  _listeners[name].push(callback);
}

exports.removeEventListener = function(name, callback) {
  if (typeof name !== "string" || typeof callback !== "function") {
    console.error("Invalid parameter for presentation.removeEventListener!");
    return;
  }

  if (_listeners[name]) {
    var index = _listeners[name].indexOf(callback);
    if (index != -1)
      _listeners[name].splice(index, 1);
  }
}

// APIs
exports.startSession = function(presentationUrl) {
  return new Promise(function(resolve, reject) {
    var requestId = ++_nextRequestId;
    var request = new SessionRequest(requestId, resolve, reject);
    _sessionRequests[requestId] = request;

    var baseUrl = location.protocol + "//" + location.host;
    var message = { "cmd": "StartSession", "requestId": requestId, "url": presentationUrl, "baseUrl": baseUrl };

    extension.postMessage(JSON.stringify(message));
  });
}

exports.joinSession = function(url, presentationId) {
  return new Promise(function(resolve, reject) {
    // TODO: to be implemented.
    console.warn("TODO: joinSession need to be implemented");
    reject(null);
  });
}

exports.getAvailability = function() {
  return new Promise(function(resolve, reject) {
    var res = extension.internal.sendSyncMessage("GetAvailability");
    var availability = new Availability();
    availability.value = (res == "true" ? true : false);
    resolve(availability);
  });
}

// Native message handlers
function handleAvailabilityChange(isAvailable) {
  if (_displayAvailable == isAvailable)
    return;

  _displayAvailable = isAvailable;
  if (!_listeners[CHANGE_EVENT])
    return;

  var length = _listeners[CHANGE_EVENT].length;
  for (var i = 0; i < length; ++i) {
    _listeners[CHANGE_EVENT][i].apply(null, null);
  }
}

function handleSessionStartSucceeded(requestId, viewId) {
  var request = _sessionRequests[requestId];
  if (request) {
    var view = v8toolsNative.getWindowObject(viewId);
    var session = new Session(viewId);
    request._successCallback.apply(null, [session]);
    delete _sessionRequests[requestId];
  }
}

function handleSessionStartFailed(requestId, errorMessage) {
  var request = _sessionRequests[requestId];
  if (request) {
    var error = new DOMError(errorMessage);
    if (request._errorCallback)
      request._errorCallback.apply(null, [error]);
    delete _sessionRequests[requestId];
  }
}

function handleDefaultSessionStarted(requestId, viewId) {
  // TODO: to be implemented.
  console.warn("TODO: DefaultSesionStarted need to be implemented")
}

function handleSessionMessageToHostReceived(presentationId, data) {
  var event = new Event('messagetohost');
  event.data = data;
  dispatchEvent(event);
}

function handleSessionMessageToRemoteReceived(presentationId, data) {
  var event = new Event('messagetoremote');
  event.data = data;
  dispatchEvent(event);
}

extension.setMessageListener(function(json) {
  var msg = JSON.parse(json);
  if (msg.cmd == "AvailabilityChange") {
    /* Using setTimeout here to ensure the error in user-defined event handler
       would be captured in developer tools. */
    setTimeout(function() {
      handleAvailabilityChange(msg.data);
    }, 0);
  } else if (msg.cmd == "SessionStartSucceeded") {
    setTimeout(function() {
      handleSessionStartSucceeded(msg.requestId, parseInt(msg.data) /* view id */);
    }, 0);
  } else if (msg.cmd == "SessionStartFailed") {
    setTimeout(function() {
      handleSessionStartFailed(msg.requestId, msg.data /* error message */);
    }, 0);
  } else if (msg.cmd == "DefaultSessionStarted") {
    setTimeout(function() {
      handleDefaultSessionStarted(msg.requestId, parseInt(msg.data) /* view id */);
    }, 0);
  } else if (msg.cmd == "SessionMessageToHostReceived") {
    handleSessionMessageToHostReceived(msg.presentationId, msg.data);
  } else if (msg.cmd == "SessionMessageToRemoteReceived") {
    handleSessionMessageToRemoteReceived(msg.presentationId, msg.data);
  } else {
    console.error("Invalid message : " + msg.cmd);
  }
});
