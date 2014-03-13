// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var application = requireNative('application');
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
  var _keyList = new Array();

  this.init = function() {
    var result = extension.internal.sendSyncMessage(
        { cmd: 'GetAllItems' });
    for (var itemKey in result) {
      var itemValue = result[itemKey];
      this[itemKey] = itemValue;
      _keyList.push(itemKey);
    }
  }

  this.__defineGetter__('length', function() {
    return _keyList.length;
  });

  this.key = function(index) {
    return _keyList[index];
  }

  this.getItem = function(itemKey) {
    return this[String(itemKey)];
  } 

  this.setItem = function(itemKey, itemValue) {
    var result = extension.internal.sendSyncMessage({
        cmd: 'SetPreferencesItem',
        preferencesItemKey: String(itemKey),
        preferencesItemValue: String(itemValue) });

    if (result) {
      this[String(itemKey)] = String(itemValue);
      _keyList.push(String(itemKey));
    } else {
      throw new common.CustomDOMException(
          common.CustomDOMException.NO_MODIFICATION_ALLOWED_ERR,
          'The object can not be modified.');
    }
  };
  
  this.removeItem = function(itemKey) {
    var result = extension.internal.sendSyncMessage({
        cmd: 'RemovePreferencesItem',
        preferencesItemKey: String(itemKey) });

    if (result) {
      delete this[itemKey];
      delete _keyList[_keyList.indexOf(String(itemKey))];
    } else {
      throw new common.CustomDOMException(
          common.CustomDOMException.NO_MODIFICATION_ALLOWED_ERR,
          'The object can not be modified.');
    }
  }

  this.clear = function() {
    var itemKey;
    var result = extension.internal.sendSyncMessage({
        cmd: 'ClearAllItems' });

    if (!result)
      return;

    for (var i = _keyList.length-1; i >= 0; --i) {
      // if the itemKey is still in DB(e.g. readonly),
      // we should keep it in JS side.
      var exists = extension.internal.sendSyncMessage({
          cmd: 'KeyExists',
          preferencesItemKey: _keyList[i] });

      if (!exists) {
        delete this[_keyList[i]];
        _keyList.splice(i, 1);
      }
    }
  }

  this.init();
};

var widgetStorage = new WidgetStorage();
exports.preferences = widgetStorage;

Object.defineProperty(exports, 'preferences', {
  configurable: false,
  enumerable: false,
  get: function() {
    return widgetStorage;
  }
});
