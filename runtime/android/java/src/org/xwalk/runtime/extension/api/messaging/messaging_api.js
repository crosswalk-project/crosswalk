// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// The Promise code are modified from https://gist.github.com/unscriptable/814052
// with original copyright and license as below.
//
// (c) copyright unscriptable.com / John Hann
// License MIT

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

function _isFunction( fn ) {
  return !!fn && !fn.nodeName && fn.constructor != String
    && fn.constructor != RegExp && fn.constructor != Array
    && /function/i.test( fn + "" );
}

function SmsManager(){
}

SmsManager.prototype.send = function(to, text, serviceId){
  var _msg = {
    cmd: "msg_smsSend",
    data: {
      phone: to,
      message: text,
      serviceId: serviceId
    }
  }  
  return postMessage(_msg);
}

SmsManager.prototype.segmentInfo = function(text, serviceId){
  var _msg = {
    cmd: "msg_smsSegmentInfo",
    data: {
      text: text,
      serviceId: serviceId
    }
  }
  return postMessage(_msg);
}

var sms = new SmsManager();
exports.sms = sms;

function handleReceived(msgObj){
  switch(msgObj.data.message.type) {
    case "sms": {
        if (_isFunction(sms.onreceived)) {
          sms.onreceived(msgObj.data)
        }
      }
      break;
    case "mms": {
        //FIXME:(shawn) waiting for mms ready
      }
      break;
    default:
      break;
  
  }
}

function handleSmsDelivery(msgObj){
  if (!msgObj.data.error) {
    if (_isFunction(sms.ondeliverysuccess)) {
      sms.ondeliverysuccess(msgObj.data.event);
    }
  } else {
    if (_isFunction(sms.ondeliveryerror)) {
      sms.ondeliveryerror(msgObj.data.event);
    }
  }
}

function MessagingCursor(element){
  this.messageIndex = 0;
  this.element = element;
}

MessagingCursor.prototype.next = function(){
  var ret = null
  if (this.messageIndex>this.element.length) { 
    this.messageIndex = this.element.length;
    ret = null;
  } else {
    if (this.element) {
      ret = this.element[this.messageIndex];
      this.messageIndex++;
    };
  }
  return ret;
}

MessagingCursor.prototype.previous = function(){
  var ret = null
  if (this.messageIndex<0) { 
    this.messageIndex = 0;
    ret = null;
  } else {
    if (this.element) {
      ret = this.element[this.messageIndex];
      this.messageIndex--;
    };
  }
  return ret;
}

function handleSent(msgObj){
  if (msgObj.data.error) {
    _promises[msgObj._promise_id].reject(msgObj.data.body);
  } else {
    if (_isFunction(sms.onsent)) {
      var event = {
        message: msgObj.data.body,
      }
      sms.onsent(event);
    }
    _promises[msgObj._promise_id].fulfill(msgObj.data.body);
  }

  delete _promises[msgObj._promise_id];
}

function handleFindMessages(msgObj){
  if (msgObj.data.error) {
    _promises[msgObj._promise_id].reject(msgObj.data.body);
  } else {
    var cursor = new MessagingCursor(msgObj.data.body.results);
    _promises[msgObj._promise_id].fulfill(cursor);
  }

  delete _promises[msgObj._promise_id];
}

function handlePromise(msgObj){
  if (msgObj.data.error) {
    _promises[msgObj._promise_id].reject(msgObj.data.body);
  } else {
    _promises[msgObj._promise_id].fulfill(msgObj.data.body);
  }

  delete _promises[msgObj._promise_id];
}

extension.setMessageListener(function(json) {
  var _msg = JSON.parse(json);

  switch (_msg.cmd) {
    case "msg_smsDeliver": {
      handleSmsDelivery(_msg);
      break;
    }
    case "msg_smsReceived":{
      handleReceived(_msg);
      break;
    }
    case "msg_findMessages_ret":{
      handleFindMessages(_msg);
      break;
    }
    case "msg_smsSend_ret":{
      handleSent(_msg);
      break;
    }
    case "msg_smsSegmentInfo_ret":
    case "msg_getMessage_ret":
    case "msg_deleteMessage_ret":
    case "msg_deleteConversation_ret":
    case "msg_markMessageRead_ret":
    case "msg_markConversationRead_ret":{
      handlePromise(_msg);
      break;
    }
    default:
      break;
  }
});

exports.findMessages = function(filter, options){
  var _msg = {
    cmd: "msg_findMessages",
    data: {
      filter: filter,
      options: options
    }
  }
  return postMessage(_msg);
}

exports.getMessage = function(type, messageID){
  var _msg = {
    cmd: "msg_getMessage",
    data: {
      type: type,
      messageID: messageID
    }
  }
  return postMessage(_msg);
}

exports.deleteMessage = function(type, messageID){
    var _msg = {
    cmd: "msg_deleteMessage",
    data: {
      type: type,
      messageID: messageID
    }
  }
  return postMessage(_msg);
}

exports.deleteConversation = function(type, conversationID){
    var _msg = {
    cmd: "msg_deleteConversation",
    data: {
      type: type,
      conversationID: conversationID
    }
  }
  return postMessage(_msg);
}

exports.markMessageRead = function(type, messageID, value){
  value = arguments[2] != undefined ? arguments[2] : true;
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

exports.markConversationRead = function(type, conversationID, value){
  value = arguments[2] != undefined ? arguments[2] : true;
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
