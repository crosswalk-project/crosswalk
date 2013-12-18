// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var _promises = {};
var _next_promise_id = 0;
var _listeners = {};
// Preserve 6 spaces to hold onreceived, onsent, ondeliverysuccess,
// ondeliveryerror, onserviceadded and onserviceremoved's
// callback functions.
var _next_listener_id = 6;

var Promise = requireNative('sysapps_promise').Promise;

var postMessage = function(msg) {
  var p = new Promise();

  _promises[_next_promise_id] = p;
  msg._promise_id = _next_promise_id.toString();
  _next_promise_id += 1;

  extension.postMessage(JSON.stringify(msg));
  return p;
};

function _isFunction(fn) {
  return !!fn && !fn.nodeName && fn.constructor != String
    && fn.constructor != RegExp && fn.constructor != Array
    && /function/i.test( fn + "" );
}

function SmsManager() {}

SmsManager.prototype.type = "sms";

Object.defineProperty(SmsManager.prototype, 'serviceIDs', {
  get: function() {
    var _msg = {
      cmd: "msg_smsServiceId",
    }
    return JSON.parse(extension.internal.sendSyncMessage(JSON.stringify(_msg)));
  }
});

SmsManager.prototype.send = function(to, text, serviceID) {
  var _msg = {
    cmd: "msg_smsSend",
    data: {
      phone: to,
      message: text,
      serviceID: serviceID
    }
  }
  return postMessage(_msg);
}

SmsManager.prototype.clear = function(serviceID) {
  var _msg = {
    cmd: "msg_smsClear",
    data: {
      serviceID: serviceID
    }
  }  
  return postMessage(_msg);
}

SmsManager.prototype.segmentInfo = function(text, serviceID) {
  var _msg = {
    cmd: "msg_smsSegmentInfo",
    data: {
      text: text,
      serviceID: serviceID
    }
  }
  return postMessage(_msg);
}

var sms = new SmsManager();
exports.sms = sms;

Object.defineProperty(exports.sms, 'onreceived', {
  set: function(callback) {
    _addEventListener(true, 'received', callback);
  }
});

Object.defineProperty(exports.sms, 'onsent', {
  set: function(callback) {
    _addEventListener(true, 'sent', callback);
  }
});

Object.defineProperty(exports.sms, 'ondeliverysuccess', {
  set: function(callback) {
    _addEventListener(true, 'deliverysuccess', callback);
  }
});

Object.defineProperty(exports.sms, 'ondeliveryerror', {
  set: function(callback) {
    _addEventListener(true, 'deliveryerror', callback);
  }
});

Object.defineProperty(exports.sms, 'onserviceadded', {
  set: function(callback) {
    _addEventListener(true, 'serviceadded', callback);
  }
});

Object.defineProperty(exports.sms, 'onserviceremoved', {
  set: function(callback) {
    _addEventListener(true, 'serviceremoved', callback);
  }
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
      case 'received':
        _listeners[0] = listener;
        listener_id = 0;
        break;
      case 'sent':
        _listeners[1] = listener;
        listener_id = 1;
        break;
      case 'deliverysuccess':
        _listeners[2] = listener;
        listener_id = 2;
        break;
      case 'deliveryerror':
        _listeners[3] = listener;
        listener_id = 3;
        break;
      case 'serviceadded':
        _listeners[4] = listener;
        listener_id = 4;
        break;
      case 'serviceremoved':
        _listeners[5] = listener;
        listener_id = 5;
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

  return listener_id;
}

exports.addEventListener = function(eventName, callback) {
  return _addEventListener(false, eventName, callback);
}

function handleEvent(msg) {
  for (var id in _listeners) {
    if (_listeners[id]['eventName'] === msg.cmd && _listeners[id]['callback']) {
      _listeners[id]['callback'](msg.data);
    }
  }
}

function MessagingCursor(element) {
  this.messageIndex = 0;
  this.element = element;
}

MessagingCursor.prototype.next = function() {
  var ret = null;
  if (this.messageIndex > this.element.length) { 
    this.messageIndex = this.element.length;
  } else {
    if (this.element) {
      ret = this.element[this.messageIndex];
      this.messageIndex++;
    }
  }
  return ret;
}

MessagingCursor.prototype.previous = function() {
  var ret = null;
  if (this.messageIndex < 0) { 
    this.messageIndex = 0;
  } else {
    if (this.element) {
      ret = this.element[this.messageIndex];
      this.messageIndex--;
    }
  }
  return ret;
}

function handleFindMessages(msgObj) {
  if (msgObj.data.error) {
    if (_isFunction(_promises[msgObj._promise_id].reject)) {
      _promises[msgObj._promise_id].reject(msgObj.data.body);
    }
  } else {
    if (_isFunction(_promises[msgObj._promise_id].fulfill)) {
      var cursor = new MessagingCursor(msgObj.data.body.results);
      _promises[msgObj._promise_id].fulfill(cursor);
    }
  }

  delete _promises[msgObj._promise_id];
}

function handlePromise(msgObj) {
  if (msgObj.data.error) {
    if (_isFunction(_promises[msgObj._promise_id].reject)) {
      _promises[msgObj._promise_id].reject(msgObj.data.body);
    }
  } else {
    if (_isFunction(_promises[msgObj._promise_id].fulfill)) {
      _promises[msgObj._promise_id].fulfill(msgObj.data.body);
    }
  }

  delete _promises[msgObj._promise_id];
}

extension.setMessageListener(function(json) {
  var _msg = JSON.parse(json);

  switch (_msg.cmd) {
    case "sent":
    case "received":
    case "deliverysuccess":
    case "deliveryerror":
    case "serviceadded":
    case "serviceadded": {
      handleEvent(_msg);
      break;
    }
    case "msg_findMessages_ret": {
      handleFindMessages(_msg);
      break;
    }
    case "msg_smsSend_ret":
    case "msg_smsClear_ret":
    case "msg_smsSegmentInfo_ret":
    case "msg_getMessage_ret":
    case "msg_deleteMessage_ret":
    case "msg_deleteConversation_ret":
    case "msg_markMessageRead_ret":
    case "msg_markConversationRead_ret": {
      handlePromise(_msg);
      break;
    }
    default:
      break;
  }
});

exports.findMessages = function(filter, options) {
  var _msg = {
    cmd: "msg_findMessages",
    data: {
      filter: filter,
      options: options
    }
  }
  return postMessage(_msg);
}

exports.findConversations = function(groupBy, filter, options) {
  // TODO:(shawn) Spec is not ready for this part.
}

exports.getMessage = function(type, messageID) {
  var _msg = {
    cmd: "msg_getMessage",
    data: {
      type: type,
      messageID: messageID
    }
  }
  return postMessage(_msg);
}

exports.deleteMessage = function(type, messageID) {
  var _msg = {
    cmd: "msg_deleteMessage",
    data: {
      type: type,
      messageID: messageID
    }
  }
  return postMessage(_msg);
}

exports.deleteConversation = function(type, conversationID) {
  var _msg = {
    cmd: "msg_deleteConversation",
    data: {
      type: type,
      conversationID: conversationID
    }
  }
  return postMessage(_msg);
}

exports.markMessageRead = function(type, messageID, value) {
  value = (typeof value === 'undefined') ? true : value;
  var _msg = {
    cmd: "msg_markMessageRead",
    data: {
      type: type,
      messageID: messageID,
      value: value
    }
  }
  return postMessage(_msg);
}

exports.markConversationRead = function(type, conversationID, value) {
  value = (typeof value === 'undefined') ? true : value;
  var _msg = {
    cmd: "msg_markConversationRead",
    data: {
      type: type,
      conversationID: conversationID,
      value: value
    }
  }
  return postMessage(_msg);
}
