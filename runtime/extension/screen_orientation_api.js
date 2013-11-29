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

// FIXME(zliang7): It should fire an initial orientation event
// rather than define a default value here.
exports.orientation = "portrait-primary";
exports.onorientationchange = null;

exports.lock = function(orientations) {
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

exports.unlock = function() {
  exports.lock(["portrait", "landscape"]);
};

exports.listeners = [];

exports.addEventListener = function(type, listener) {
  if (type == "orientationchange" && listener instanceof Function &&
      exports.listeners.indexOf(listener) < 0) {
    exports.listeners.push(listener);
  }
}

exports.removeEventListener = function(type, listener) {
  if (type == "orientationchange" && listener instanceof Function) {
    var index = exports.listeners.indexOf(listener);
    if (index >= 0)
      exports.listeners.splice(index, 1);
  }
}

exports.dispatchEvent = function(event) {
  exports.listeners.forEach(function(listener) {
    try {
      listener(event);
    } catch (e) {
      // discard exceptions if any
    }
  });

  if (exports.onorientationchange) {
    try {
      exports.onorientationchange(event);
    } catch (e) {
      // discard exceptions if any
    }
  }
}

function messageListener(value) {
  switch (value) {
    case PORTRAIT_PRIMARY:
      exports.orientation = "portrait-primary";
      break;
    case PORTRAIT_SECONDARY:
      exports.orientation = "portrait-secondary";
      break;
    case LANDSCAPE_PRIMARY:
      exports.orientation = "landscape-primary";
      break;
    case LANDSCAPE_SECONDARY:
      exports.orientation = "landscape-secondary";
      break;
    default:
      console.error("Internal error");
      return;
  }
  var event = new Event("orientationchange");
  exports.dispatchEvent(event)
}


Object.defineProperty(window.screen, "orientation", {
  configurable: false,
  enumerable: true,
  get: function() { return exports.orientation; }
});

Object.defineProperty(window.screen, "onorientationchange", {
  configurable: false,
  enumerable: true,
  get: function() { return exports.onorientationchange; },
  set: function(val) {
    if (val == null || val instanceof Function)
      exports.onorientationchange = val;
  }
});

window.screen.lockOrientation = exports.lock;
window.screen.unlockOrientation = exports.unlock;
window.screen.addEventListener = exports.addEventListener;
window.screen.removeEventListener = exports.removeEventListener;
window.screen.dispatchEvent = exports.dispatchEvent;

extension.setMessageListener(messageListener);
