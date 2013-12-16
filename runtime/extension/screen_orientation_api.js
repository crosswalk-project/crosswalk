// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var internal = requireNative("internal");
internal.setupInternalExtension(extension);

// These constants must correspond with C++ side.
var PORTRAIT_PRIMARY    = 0x01;
var PORTRAIT_SECONDARY  = 0x02;
var LANDSCAPE_PRIMARY   = 0x04;
var LANDSCAPE_SECONDARY = 0x08;
var PORTRAIT            = PORTRAIT_PRIMARY | PORTRAIT_SECONDARY;
var LANDSCAPE           = LANDSCAPE_PRIMARY | LANDSCAPE_SECONDARY;

var screen_orientation = null;
var screen_onorientationchange = null;

extension.setMessageListener(function (value) {
  var is_initial = !screen_orientation;
  switch (value) {
    case PORTRAIT_PRIMARY:
      screen_orientation = "portrait-primary";
      break;
    case PORTRAIT_SECONDARY:
      screen_orientation = "portrait-secondary";
      break;
    case LANDSCAPE_PRIMARY:
      screen_orientation = "landscape-primary";
      break;
    case LANDSCAPE_SECONDARY:
      screen_orientation = "landscape-secondary";
      break;
    default:
      console.error("Internal error");
      return;
  }

  if (!is_initial) {
    var event = new Event("orientationchange");
    window.screen.dispatchEvent(event)
  }
});

window.screen.lockOrientation = function(orientations) {
  if (!Array.isArray(orientations)) {
    if (typeof(orientations) != "string")
      return false;
    orientations = [orientations];
  }

  var value = 0;
  for (var i = 0; i < orientations.length; ++i) {
    switch (orientations[i]) {
      case "portrait-primary":
        value |= PORTRAIT_PRIMARY;
        break;
      case "portrait-secondary":
        value |= PORTRAIT_SECONDARY;
        break;
      case "landscape-primary":
        value |= LANDSCAPE_PRIMARY;
        break;
      case "landscape-secondary":
        value |= LANDSCAPE_SECONDARY;
        break;
      case "portrait":
        value |= PORTRAIT;
        break;
      case "landscape":
        value |= LANDSCAPE;
        break;
      default:
        console.error("Invalid screen orientation");
        return false;
    }
  }
  internal.postMessage('lock', [value], null);
  return true;
};

window.screen.unlockOrientation = function() {
  window.screen.lockOrientation(["portrait", "landscape"]);
};

// Simple implementation of EventTarget interface

var screen_listeners = [];

window.screen.addEventListener = function(type, listener) {
  if (type == "orientationchange" &&
      (listener instanceof Function ||
       listener.handleEvent instanceof Function) &&
      screen_listeners.indexOf(listener) < 0) {
    screen_listeners.push(listener);
  }
}

window.screen.removeEventListener = function(type, listener) {
  if (type == "orientationchange") {
    var index = screen_listeners.indexOf(listener);
    if (index >= 0)
      screen_listeners.splice(index, 1);
  }
}

window.screen.dispatchEvent = function(event) {
  screen_listeners.forEach(function(listener) {
    try {
      if (listener.handleEvent instanceof Function)
        listener.handleEvent.call(window.screen, event);
      else
        listener.call(window.screen, event);
    } catch (e) {
      // discard exceptions if any
      // http://www.w3.org/TR/DOM-Level-3-Events/#event-flow
    }
  });

  if (screen_onorientationchange) {
    try {
      screen_onorientationchange.call(window.screen, event);
    } catch (e) {
      // discard exceptions if any
    }
  }
}


Object.defineProperty(window.screen, "orientation", {
  configurable: false,
  enumerable: true,
  get: function() { return screen_orientation; }
});

Object.defineProperty(window.screen, "onorientationchange", {
  configurable: false,
  enumerable: true,
  get: function() { return screen_onorientationchange; },
  set: function(val) {
    if (val == null || val instanceof Function)
      screen_onorientationchange = val;
  }
});
