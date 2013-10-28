/**
 * Copyright (c) 2013 Intel Corporation. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

var DISPLAY_AVAILABLE_CHANGE_EVENT = "displayavailablechange";
var _listeners = {};
var _displayAvailable = false;

function addEventListener(name, callback, useCapture /* ignored */) {
  if (typeof name !== "string" || typeof callback !== "function") {
    console.error("Invalid parameter for presentation.addEventListener!");
    return;
  }

  if (!_listeners[name])
  	_listeners[name] = [];
  _listeners[name].push(callback);
}

function removeEventListener(name, callback) {
  if (typeof name !== "string" || typeof callback !== "function") {
    console.error("Invalid parameter for presentation.removeEventListener!");
    return;
  }

  if (_listeners[name]) {
  	var index = _listeners[name].indexOf(callback);
  	if (index != -1)
  	  _listeners[name].splice(index, 1);
  }
}

function handleDisplayAvailableChange(isAvailable) {
  if (_displayAvailable == isAvailable)
    return;

  _displayAvailable = isAvailable;
  if (!_listeners[DISPLAY_AVAILABLE_CHANGE_EVENT])
    return;

  var length = _listeners[DISPLAY_AVAILABLE_CHANGE_EVENT].length;
  for (var i = 0; i < length; ++i) {
    _listeners[DISPLAY_AVAILABLE_CHANGE_EVENT][i].apply(null, null);
  }
}

extension.setMessageListener(function(json) {
  var msg = JSON.parse(json);
  if (msg.cmd == "DisplayAvailableChange") {
    /* Using setTimeout here to ensure the error in user-defined event handler
       would be captured in developer tools. */
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
  /* If there is at least one event listener installed, we can safely use the
     _displayAvailable flag. Otherwise, we need to send a message to query it */
  if (!_listeners[DISPLAY_AVAILABLE_CHANGE_EVENT] ||
      _listeners[DISPLAY_AVAILABLE_CHANGE_EVENT].length == 0) {
    var res = extension.internal.sendSyncMessage("QueryDisplayAvailability");
    _displayAvailable = (res == "true" ? true : false);
  }
  return _displayAvailable;
});
