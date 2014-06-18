// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var internal = requireNative('internal');
internal.setupInternalExtension(extension);

// These constants must correspond with C++ side:
// xwalk/runtime/browser/ui/screen_orientation.h
var PORTRAIT_PRIMARY    = 1 << 0;
var LANDSCAPE_PRIMARY   = 1 << 1;
var PORTRAIT_SECONDARY  = 1 << 2;
var LANDSCAPE_SECONDARY = 1 << 3;
var PORTRAIT            = PORTRAIT_PRIMARY | PORTRAIT_SECONDARY;
var LANDSCAPE           = LANDSCAPE_PRIMARY | LANDSCAPE_SECONDARY;
var ANY                 = PORTRAIT | LANDSCAPE;

// Values that can be set externally.
var uaDefault = uaDefault || ANY;
var isAndroid = isAndroid || false;

// Store the real Screen object.
var realScreen = window.screen;

function cloneProperties(target, source) {
  var props = Object.getOwnPropertyNames(source);
  props.forEach(function(item) {
    target[item] = source[item];
  });
}

// Create replacement Screen and make it an EventTarget.
function Screen() { cloneProperties(this, realScreen); };
Screen.prototype = Object.create(EventTarget.prototype);
Screen.prototype.constructor = Screen;

// Replace it.
window.screen = new Screen();

function postMessage(command, value) {
  // Currently, internal.postMessage can't work on Android.
  // https://crosswalk-project.org/jira/browse/XWALK-855
  if (isAndroid) {
    var message = JSON.stringify({
      cmd: command,
      value: value.toString()
    });
    extension.postMessage(message);
  } else {
    internal.postMessage(command, value, null);
  }
}

window.screen.lockOrientation = function(orientations) {
  if (!Array.isArray(orientations)) {
    if (typeof orientations != 'string')
      return false;
    orientations = [orientations];
  }

  var value = 0;
  for (var i = 0; i < orientations.length; ++i) {
    switch (orientations[i]) {
      case 'portrait-primary':
        value |= PORTRAIT_PRIMARY;
        break;
      case 'portrait-secondary':
        value |= PORTRAIT_SECONDARY;
        break;
      case 'landscape-primary':
        value |= LANDSCAPE_PRIMARY;
        break;
      case 'landscape-secondary':
        value |= LANDSCAPE_SECONDARY;
        break;
      case 'portrait':
        value |= PORTRAIT;
        break;
      case 'landscape':
        value |= LANDSCAPE;
        break;
      default:
        console.error('Invalid screen orientation');
        return false;
    }
    // If the orientations aren't all part of the default allowed
    // orientations, the steps must stop here and return false.
    if ((uaDefault & value) != value)
      return false;
  }
  postMessage('lock', [value]);
  return true;
};

window.screen.unlockOrientation = function() {
  postMessage('lock', [uaDefault]);
};

// Create a HTMLUnknownElement and do not attach it to the DOM.
var dispatcher = document.createElement('xwalk-EventDispatcher');

// Implement EventTarget interface on object.
Object.defineProperty(window.screen, 'addEventListener', {
  value: function(type, callback, capture) {
    dispatcher.addEventListener(type, callback, capture);
  }
});

Object.defineProperty(window.screen, 'removeEventListener', {
  value: function(type, callback, capture) {
    dispatcher.removeEventListener(type, callback, capture);
  }
});

Object.defineProperty(window.screen, 'dispatchEvent', {
  value: function(e) {
    dispatcher.dispatchEvent(e);
  }
});

var orientationchangeCallback = null;
var orientationchangeCallbackWrapper = null;

Object.defineProperty(window.screen, 'onorientationchange', {
  configurable: false,
  enumerable: true,
  get: function() { return orientationchangeCallback; },
  set: function(callback) {
    // We must add the on* event as an event listener so that
    // it is called at the right point between potential
    // event listeners, but it cannot be the exact method
    // as that would allow removeEventListener to remove it.

    // Remove existing (wrapped) listener.
    window.screen.removeEventListener('orientationchange',
        orientationchangeCallbackWrapper);

    // If valid, store and add a wrapped version as listener.
    if (callback instanceof Function) {
      orientationchangeCallback = callback;
      orientationchangeCallbackWrapper = function() { callback(); };
      window.screen.addEventListener('orientationchange',
          orientationchangeCallbackWrapper);
    }
    // If not valid, reset to null.
    else {
      orientationchangeCallback = null;
      orientationchangeCallbackWrapper = null;
    }
  }
});

function handleOrientationChange(newOrientation) {
  switch (newOrientation) {
    case PORTRAIT_PRIMARY:
      orientationValue = 'portrait-primary';
      break;
    case PORTRAIT_SECONDARY:
      orientationValue = 'portrait-secondary';
      break;
    case LANDSCAPE_PRIMARY:
      orientationValue = 'landscape-primary';
      break;
    case LANDSCAPE_SECONDARY:
      orientationValue = 'landscape-secondary';
      break;
    default:
      console.error('Received unknown value for current orientation');
      return;
  }

  // The first time the listener is called it is to set the current
  // orientation, so do not dispatch the orientationchange in that case.
  if (handleOrientationChange.shouldDispatchEvent) {
    var event = new Event('orientationchange');
    dispatcher.dispatchEvent(event);
  }

  handleOrientationChange.shouldDispatchEvent = true;
}

var orientationValue;

Object.defineProperty(window.screen, 'orientation', {
  configurable: false,
  enumerable: true,
  get: function() {
    if (typeof orientationValue == 'undefined') {
      var msg = JSON.stringify({
        cmd: 'GetScreenOrientation'
      });
      var newOrientation = extension.internal.sendSyncMessage(msg);
      handleOrientationChange(newOrientation);
    }
    return orientationValue;
  }
});

// FIXME: Extend message listener to handle screen changes.
extension.setMessageListener(handleOrientationChange);
