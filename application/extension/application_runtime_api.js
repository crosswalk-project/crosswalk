// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var application = requireNative('application');

var internal = requireNative("internal");
internal.setupInternalExtension(extension);

exports.getManifest = function(callback) {
  internal.postMessage('getManifest', [], callback);
}

exports.getMainDocument = function(callback) {
  internal.postMessage('getMainDocumentID', [], function(routing_id) {
    var md = application.getViewByID(routing_id);
    callback(md);
  });
}
