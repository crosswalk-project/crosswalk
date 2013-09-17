// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var internal = requireNative("internal");
internal.setupInternalExtension(extension);

var DISPLAY_AVAILABLE_CHANGE_EVENT = "displayavailablechange";
var _listeners = {}; // Listeners of display available change event
var _displayAvailable = false;

function addEventListener(name, callback, useCapture /* ignored */) {
  if (!_listeners[name])
  	_listeners[name] = [];
  _listeners[name].push(callback);
}

function removeEventListener(name, callback) {
  if (_listeners[name]) {
  	var index = _listeners[name].indexOf(callback);
  	if (index != -1)
	  _listeners[name].splice(index, 1);
  }
}

function handleDisplayAvailableChange(isAvailable) {
	if (_displayAvailable != isAvailable) {
		_displayAvailable = isAvailable;
		if (!_listeners[DISPLAY_AVAILABLE_CHANGE_EVENT])
			return;
    var length = _listeners[DISPLAY_AVAILABLE_CHANGE_EVENT].length;
		for (var i = 0; i < length; ++i) {
			_listeners[DISPLAY_AVAILABLE_CHANGE_EVENT][i].apply(null, null);
		}
	}
}

extension.setMessageListener(function(json) {
  var msg = JSON.parse(json);
  if (msg.cmd == "DisplayAvailableChange") {
    setTimeout(function() {
      handleDisplayAvailableChange(msg.data);
    }, 0);
  } else {
    console.error("Invalid message : " + msg.cmd);
  }
})

exports.addEventListener = addEventListener;
exports.removeEventListener = removeEventListener;
exports.__defineSetter__("on" + DISPLAY_AVAILABLE_CHANGE_EVENT,
  function(callback) {
	  if (callback)
	    addEventListener(DISPLAY_AVAILABLE_CHANGE_EVENT, callback);
	  else
	    removeEventListener(DISPLAY_AVAILABLE_CHANGE_EVENT,
                          this.ondisplayavailablechange);
  }
);

exports.__defineGetter__("displayAvailable", function() {
  // If there is at least one listener registered to listen the display available
  // change, we can rely on the _displayAvailable flag. Otherwise, we need to
  // send a sync message to query the availability flag from browser process
  // each time.
  if (!_listeners[DISPLAY_AVAILABLE_CHANGE_EVENT] ||
      _listeners[DISPLAY_AVAILABLE_CHANGE_EVENT].length == 0) {
    var res = extension.internal.sendSyncMessage("QueryDisplayAvailability");
    _displayAvailable = res == "true" ? true : false;
  }
  return _displayAvailable;
});
