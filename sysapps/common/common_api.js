// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

extension._setupExtensionInternal();

var internal = extension._internal;
var v8tools = requireNative("v8tools");
var unique_id = 0;

// The BindingObject is responsible for bridging between the JavaScript
// implementation and the native code. It keeps a unique ID for each
// instance of a given object that is used by the BindingObjectStore to
// deliver messages.
//
// It also keeps track of when the instance gets collected by the Garbage
// Collector, informing the native side that the native implementation can also
// be freed.
//
// Creating a BindingObject with a predefined object ID in the constructor can
// be used for having one or more JavaScript objects communicating with a native
// object of the same ID, but only the original BindingObject that generated the
// ID will have its lifecycle bound to the native object.
//
// _postMessage(function_name, arguments, callback):
//     This method sends a message to the native counterpart of this
//     object. It has the same signature of the Internal Extensions
//     |postMessage| but wraps the unique identifier automatically.
//
// _addMethod(name):
//     Convenience function for adding methods to an object that have a
//     correspondent on the native side. Methods names that start with "_" are
//     define as not enumerable by default.
//
var BindingObjectPrototype = function() {
  function postMessage(name, args, callback) {
    return internal.postMessage("postMessageToObject",
        [this._id, name, args], callback);
  };

  function addMethod(name) {
    var enumerable = name.indexOf("_") != 0;
    Object.defineProperty(this, name, {
      value: function() {
        this._postMessage(name, Array.prototype.slice.call(arguments));
      },
      enumerable: enumerable,
    });
  };

  Object.defineProperties(this, {
    "_postMessage" : {
      value: postMessage,
    },
    "_addMethod" : {
      value: addMethod,
    },
  });
};

var BindingObject = function(object_id) {
  Object.defineProperties(this, {
    "_id": {
      value: object_id ? object_id : (unique_id++).toString(),
    },
    "_tracker": {
      value: object_id ? undefined : v8tools.lifecycleTracker(),
    },
  });

  if (this._tracker == undefined)
    return;

  object_id = this._id;

  this._tracker.destructor = function() {
    internal.postMessage("destroyObject", [object_id]);
  };
};

// This class implements the W3C EventTarget interface and also offer
// convenience methods for declaring events. The native implementation class is
// expected to inherit from sysapps::EventTarget.
//
// The following interface will be always publicly available for every object
// using this prototype and they behave just like the specified:
//
// addEventListener(type, listener)
// removeEventListener(type, listener)
// dispatchEvent(event)
//
// The following method is available for internal usage only:
//
// _addEvent(event_name, EventSynthesizer?):
//     Convenience function for declaring the events available for the
//     EventTarget. It will also declare a functional on[type] EventHandler.
//     The optional EventSynthesizer, if supplied, will be used for create
//     the event, if not supplied, a default MessageEvent is created (the data
//     is simply associated to event.data).
//
// Important considerations:
//    - Objects with message listeners attached are never going to be collected
//      by the garbage collector (which is fine and expected).
//    - That said, an object listening for its own events is going to leak. It
//      can be solved by creating a proxy object (see TCPSocket).
//
var EventTargetPrototype = function() {
  var DefaultEvent = function(type, data) {
    this.type = type;

    if (data)
      this.data = data;
  };

  function addEvent(type, event) {
    Object.defineProperty(this, "_on" + type, {
      writable : true,
    });

    Object.defineProperty(this, "on" + type, {
      get: function() {
        return this["_on" + type];
      },
      set: function(listener) {
        var old_listener = this["_on" + type];
        if (old_listener === listener)
          return;

        if (old_listener)
          this.removeEventListener(type, old_listener);

        this["_on" + type] = listener;
        this.addEventListener(type, listener);
      },
      enumerable: true,
    });

    if (event)
      this._event_synthesizers[type] = event;
    else
      this._event_synthesizers[type] = DefaultEvent;
  };

  function dispatchEvent(event) {
    if (!event.type)
      return;
    if (!(event.type in this._event_listeners))
      return;

    var listeners = this._event_listeners[event.type];
    for (var i in listeners)
      listeners[i](event);
  };

  function dispatchEventFromExtension(type, data) {
    var listeners = this._event_listeners[type];

    for (var i in listeners)
      listeners[i](new this._event_synthesizers[type](type, data));
  };

  // We need a reference to the calling object because
  // this function is called by the renderer process with
  // "this" equals to the global object.
  function makeCallbackListener(obj, type) {
    return function(data) {
      obj._dispatchEventFromExtension(type, data);
      return true;
    };
  };

  function addEventListener(type, listener) {
    if (!(listener instanceof Function))
      return;

    if (!(("on" + type) in this))
      return;

    if (type in this._event_listeners) {
      var listeners = this._event_listeners[type];
      if (!(listener in listeners))
        listeners.push(listener);
    } else {
      this._event_listeners[type] = [listener];
      var id = this._postMessage("addEventListener",
          [type], makeCallbackListener(this, type));
      this._callback_listeners_id[type] = id;
    }
  };

  function removeEventListener(type, listener) {
    if (!(listener instanceof Function))
      return;

    if (!(type in this._event_listeners))
      return;

    var listeners = this._event_listeners[type];
    var index = listeners.indexOf(listener);
    if (index == -1)
      return;

    if (listeners.length == 1) {
      internal.removeCallback(this._callback_listeners_id[type]);
      delete this._event_listeners[type];
      delete this._callback_listeners_id[type];
      this._postMessage("removeEventListener", [type]);
    } else {
      this._event_listeners[type] = listeners.splice(index, 1);
    }
  };

  Object.defineProperties(this, {
    "_addEvent" : {
      value : addEvent,
    },
    "_dispatchEventFromExtension" : {
      value : dispatchEventFromExtension,
    },
    "addEventListener" : {
      value : addEventListener,
      enumerable : true,
    },
    "removeEventListener" : {
      value : removeEventListener,
      enumerable : true,
    },
    "dispatchEvent" : {
      value : dispatchEvent,
      enumerable : true,
    },
  });
};

var EventTarget = function(object_id) {
  Object.defineProperties(this, {
    "_event_listeners": {
      value: {},
    },
    "_callback_listeners_id": {
      value: {},
    },
    "_event_synthesizers": {
      value: {},
    },
  });
};

EventTargetPrototype.prototype = new BindingObjectPrototype();
