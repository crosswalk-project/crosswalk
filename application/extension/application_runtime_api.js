// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var internal = requireNative("internal");
internal.setupInternalExtension(extension);

var application = requireNative('application');

// Exported methods.
exports.getManifest = function(callback) {
  internal.postMessage('getManifest', [], callback);
};
