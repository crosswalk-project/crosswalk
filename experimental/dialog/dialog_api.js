// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

xwalk.experimental = xwalk.experimental || {};

(function() {

  var _callbacks = {};
  var _next_reply_id = 0;

  var postMessage = function(msg, callback) {
    var reply_id = _next_reply_id;
    _next_reply_id += 1;
    _callbacks[reply_id] = callback;
    msg._reply_id = reply_id.toString();
    xwalk.postMessage('xwalk.experimental.dialog', JSON.stringify(msg));
  };

  xwalk.setMessageListener('xwalk.experimental.dialog', function(json) {
    var msg = JSON.parse(json);
    var reply_id = msg._reply_id;
    var callback = _callbacks[reply_id];
    if (callback) {
      delete msg._reply_id;
      delete _callbacks[reply_id];
      callback(msg);
    } else {
      console.log('Invalid reply_id received from xwalk.experimental.dialog extension: ' + reply_id);
    }
  });

  xwalk.experimental.dialog = xwalk.experimental.dialog || {};
  xwalk.experimental.dialog.showOpenDialog = function (allowMultipleSelection, chooseDirectory, title, initialPath, fileTypes, callback) {
    var msg = {
      'cmd': 'ShowOpenDialog',
      'allow_multiple_selection': allowMultipleSelection,
      'choose_directory': chooseDirectory,
      'title': title,
      'initial_path': initialPath,
      'file_types': fileTypes
    };

    postMessage(msg, function(r) {
      callback(0, r.file);
    });
  }

  xwalk.experimental.dialog.showSaveDialog = function (title, initialPath, proposedNewFilename, callback) {
    var msg = {
      'cmd': 'ShowSaveDialog',
      'title': title,
      'initial_path': initialPath,
      'proposed_name': proposedNewFilename
    };

    postMessage(msg, function(r) {
      callback(0, r.file);
    });
  }

})();
