// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

extension._setupExtensionInternal();
var internal = extension._internal;

exports.getAPIVersion = function(callback) {
  internal.postMessage('getAPIVersion', [], callback);
}
