// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var g_next_async_call_id = 0;
var g_async_calls = [];
var g_listeners = [];

// Preserve first element for "oncontactschange" way event listener.
//
// Compared with addEventListener, the main difference is there is only one
// oncontactschange callback will be invoked. While addEventListener can 
// add multiple callbacks when event "contactschanged" arrives.
//
// This listener can be set as "oncontactschange = function(){...};"
// and can be unset as "oncontactschange = null;"
g_listeners[0] = null;
var g_next_listener_id = 1;

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
    for (var id in g_listeners) {
      if (typeof g_listeners[id] === 'function') {
        g_listeners[id](_createConstClone(msg.data));
      }
    }
    return;
  }

  if (msg.data) {
    if (!msg.data.hasOwnProperty("error") || !msg.data.error) {
      g_async_calls[msg.asyncCallId].resolve(msg.data);
    } else {
      g_async_calls[msg.asyncCallId].reject(msg.data);
    }
  } else {
    console.log("WARNING: Message from backend doesn't have data.")
    g_async_calls[msg.asyncCallId].resolve();
  }

  delete g_async_calls[msg.asyncCallId];
});

exports.save = function(contact) {
  var msg = {};
  msg['cmd'] = 'save';
  msg['contact'] = contact;
  return createPromise(msg);
}

exports.find = function(options) {
  var msg = {};
  msg['cmd'] = 'find';
  msg['options'] = options;
  return createPromise(msg);
};

exports.remove = function(contactId) {
  var msg = {};
  msg['cmd'] = 'remove';
  msg['contactId'] = contactId;
  return createPromise(msg);
};

exports.clear = function() {
  var msg = {};
  msg['cmd'] = 'clear';
  return createPromise(msg);
}

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
      g_listeners[0] = callback;
      listener_id = 0;
  } else { // Set callback by addEventListner()
      listener_id = g_next_listener_id;
      ++g_next_listener_id;
      g_listeners[listener_id] = callback;
  }

  // Notify native code there is valid listener.
  if (g_listeners[0] != null || g_listeners.length > 1) {
    var msg = { 'cmd': 'addEventListener' };
    extension.postMessage(JSON.stringify(msg));
  }

  return listener_id;
}

exports.addEventListener = function(eventName, callback) {
  if (eventName !== 'contactschange') {
    console.log("Invalid parameters of eventName: "+eventName);
    return -1;
  }
  return _addListener(false, callback);
}

Object.defineProperty(exports, 'oncontactschange', {
  set: function(callback) {
    _addListener(true, callback);
  }
});

window.ContactField = function(init) {
  this.types = init.types;
  this.preferred = init.preferred;
  this.value = init.value;
};

window.ContactTelField = function(init) {
  this.carrier = init.carrier;
  this.types = init.types;
  this.preferred = init.preferred;
  this.value = init.value;
};

window.ContactAddress = function(init) {
  this.types = init.types;
  this.preferred = init.preferred;
  this.streetAddress = init.streetAddress;
  this.locality = init.locality;
  this.region = init.region;
  this.postalCode = init.postalCode;
  this.countryName = init.countryName;
};

window.ContactName = function(init) {
  this.displayName = init.displayName;
  this.honorificPrefixes = init.honorificPrefixes;
  this.givenNames = init.givenNames;
  this.additionalNames = init.additionalNames;
  this.familyNames = init.familyNames;
  this.honorificSuffixes = init.honorificSuffixes;
  this.nicknames = init.nicknames;
};

window.Contact = function(init) {
  this.id = null;
  this.lastUpdated = new Date();
  this.name = init.name;
  this.emails = init.emails;
  this.photos = init.photos;
  this.urls = init.urls;
  this.categories = init.categories;
  this.addresses = init.addresses;
  this.phoneNumbers = init.phoneNumbers;
  this.organizations = init.organizations;
  this.jobTitles = init.jobTitles;
  this.birthday = init.birthday;
  this.notes = init.notes;
  this.impp = init.impp;
  this.anniversary = init.anniversary;
  this.gender = init.gender;
};

window.ContactsChangeEvent = function(init) {
  this.added = init.added;
  this.modified = init.modified;
  this.removed = init.removed;
};
