// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var internal = requireNative("internal");
internal.setupInternalExtension(extension);

exports.showOpenDialog = function(allow_multiple_selection,
                                  choose_directories,
                                  title,
                                  initial_path,
                                  file_types,
                                  callback) {
  internal.postMessage('showOpenDialog',
                       [allow_multiple_selection,
                       choose_directories,
                       title,
                       initial_path,
                       file_types],
                       callback);
};

exports.showSaveDialog = function(title,
                                  initial_path,
                                  proposed_new_filename,
                                  callback) {
  internal.postMessage('showSaveDialog',
                       [title,
                       initial_path,
                       proposed_new_filename],
                       callback);
};
