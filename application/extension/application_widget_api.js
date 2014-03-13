// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var application = requireNative('application');

var empty = "";
var zero = 0;
var widgetStringInfo = {
  "author"      : empty,
  "description" : empty,
  "name"        : empty,
  "shortName"   : empty,
  "version"     : empty,
  "id"          : empty,
  "authorEmail" : empty,
  "authorHref"  : empty,
  "width"       : zero,
  "height"      : zero
};

function defineReadOnlyProperty(object, key, value) {
  Object.defineProperty(object, key, {
    configurable: false,
    enumerable: true,
    get: function() {
      if (key == "width") {
        value = window.innerWidth;
      } else if (key == "height") {
        value = window.innerHeight;
      } else if (value == empty) {
        value = extension.internal.sendSyncMessage(key);
      }

      return value;
    }
  });
}

for (var key in widgetStringInfo) {
  defineReadOnlyProperty(exports, key, widgetStringInfo[key]);
}

