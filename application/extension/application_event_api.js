// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var internal = requireNative("internal");
internal.setupInternalExtension(extension);

// A map of event names to the event object that is registered to that name.
var registeredEvents = {};

var Event = function(eventName) {
  this.eventName = eventName;
  this.listeners = [];
};

Event.dispatchEvent = function(eventName, args) {
  var evt = registeredEvents[eventName];
  if (!evt)
    return;

  // Make a copy of the listeners in case the listener list is modified during
  // the iteration.
  var listeners = evt.listeners.slice();
  for (var i = 0; i < listeners.length; i++) {
    try {
      listeners[i].apply(null, args);
    } catch (e) {
      console.error('Error in event handler for:' + eventName + ' :' + new Error().stack);
    }
  }

  internal.postMessage('dispatchEventFinish', [eventName]);
  return;
};

Event.prototype.addListener = function(callback) {
  if (this.listeners.length == 0) {
    var eventName = this.eventName;
    if (registeredEvents[eventName])
      throw new Error('Event:' + eventName + ' is already registered.');
    registeredEvents[eventName] = this;
    internal.postMessage('registerEvent', [eventName], function(args) {
      Event.dispatchEvent(eventName, args);
    });
  }

  if (!this.hasListener(callback))
    this.listeners.push(callback);
};

Event.prototype.removeListener = function(callback) {
  var idx = this.listeners.indexOf(callback);
  if (idx < 0)
    return;

  this.listeners.splice(idx, 1);
  if (this.listeners.length == 0) {
    if (!registeredEvents[this.eventName])
      throw new Error('Event:' + this.eventName + ' is not registered.');
    delete registeredEvents[this.eventName];
    internal.postMessage('unregisterEvent', [this.eventName]);
  }
};

Event.prototype.hasListener = function(callback) {
  return this.listeners.indexOf(callback) >= 0;
};

Event.prototype.hasListeners = function() {
  return this.listeners.length > 0;
};

exports.Event = Event;
