// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var common = requireNative('widget_common');

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
        value = extension.internal.sendSyncMessage(
            { cmd: 'GetWidgetInfo', widgetKey: key });
      }

      return value;
    }
  });
}

for (var key in widgetStringInfo) {
  defineReadOnlyProperty(exports, key, widgetStringInfo[key]);
}

var WidgetStorage = function() {
  var _SetItem = function(itemKey, itemValue) {
    var result = extension.internal.sendSyncMessage({
        cmd: 'SetPreferencesItem',
        preferencesItemKey: String(itemKey),
        preferencesItemValue: String(itemValue) });

    if (result) {
      return itemValue;
    } else {
      throw new common.CustomDOMException(
        common.CustomDOMException.NO_MODIFICATION_ALLOWED_ERR,
        'The object can not be modified.');
    }
  }

  var _GetSetter = function(itemKey) {
    var _itemKey = itemKey;
    return function(itemValue) {
      return _SetItem(_itemKey, itemValue);
    }
  }

  var _GetGetter = function(itemKey) {
    var _itemKey = itemKey;
    return function() {
      var result = extension.internal.sendSyncMessage(
          { cmd: 'GetItemValueByKey',
            preferencesItemKey: String(itemKey) });
      return result == empty ? null : result;
    }
  }

  this.init = function() {
    var result = extension.internal.sendSyncMessage({cmd: 'GetAllItems'});
    for (var itemKey in result) {
      this.__defineSetter__(String(itemKey), _GetSetter(itemKey));
      this.__defineGetter__(String(itemKey), _GetGetter(itemKey));
    }
  }

  this.__defineGetter__('length', function() {
    var result = extension.internal.sendSyncMessage({cmd: 'GetAllItems'});
    return Object.keys(result).length;
  });

  this.key = function(index) {
    var result = extension.internal.sendSyncMessage({ cmd: 'GetAllItems'});
    return Object.keys(result)[index];
  }

  this.getItem = function(itemKey) {
    var result = extension.internal.sendSyncMessage({
        cmd: 'GetItemValueByKey',
        preferencesItemKey: String(itemKey)});
    return result == empty ? null : result;
  }

  this.setItem = function(itemKey, itemValue) {
    return _SetItem(itemKey, itemValue);
  }

  this.removeItem = function(itemKey) {
    var result = extension.internal.sendSyncMessage({
        cmd: 'RemovePreferencesItem',
        preferencesItemKey: String(itemKey)});

    if (!result) {
      throw new common.CustomDOMException(
          common.CustomDOMException.NO_MODIFICATION_ALLOWED_ERR,
          'The object can not be modified.');
    }
  }

  this.clear = function() {
    extension.internal.sendSyncMessage({cmd: 'ClearAllItems'});
  }

  this.init();
};

var widgetStorage = new WidgetStorage();
exports.preferences = widgetStorage;

exports.toString = function() {
  return '[object Widget]';
}

Object.defineProperty(exports, 'preferences', {
  configurable: false,
  enumerable: false,
  get: function() {
    return widgetStorage;
  }
});
