// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Implementation of the W3C's Device Capabilities API.
// http://www.w3.org/2012/sysapps/device-capabilities/

var internal = requireNative('internal');
internal.setupInternalExtension(extension);

var v8tools = requireNative('v8tools');
var common = requireNative('sysapps_common');
common.setupSysAppsCommon(internal, v8tools);

var DeviceCapabilities = function() {
  common.BindingObject.call(this, common.getUniqueId());

  internal.postMessage("deviceCapabilitiesConstructor", [this._id]);
};

DeviceCapabilities.prototype = new common.BindingObjectPrototype();
DeviceCapabilities.prototype.constructor = DeviceCapabilities;

exports = new DeviceCapabilities();
