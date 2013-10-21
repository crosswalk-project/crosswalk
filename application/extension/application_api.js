// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

extension._setupExtensionInternal();
var internal = extension._internal;
var application = requireNative('application');

exports.getManifest = function(callback) {
  internal.postMessage('getManifest', [], callback);
}

exports.getMainDocument = function(callback) {
  internal.postMessage('getMainDocumentID', [], function(routing_id) {
    var md = application.getViewByID(routing_id);
    callback(md);
  });
}
