// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var internal = requireNative("internal");
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
var UA_DEFAULTS         = 0;

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
  internal.postMessage('lock', [UA_DEFAULTS], null);
};
