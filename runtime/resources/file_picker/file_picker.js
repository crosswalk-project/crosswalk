// Copyright (c) 2014 The Chromium Authors. All rights reserved. 
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file. 

cr.define('filePicker', function() {
  'use strict';

  function initialize() {

    var args = JSON.parse(chrome.getVariableValue('dialogArguments'));

    var input_box = document.getElementById("file-name-inputbox");
    input_box.value = args.filePath;

    $('cancel-button').addEventListener('click', function() {
        chrome.send('cancel');
        self.close();
    });
    $('cancel-button').innerText =
        loadTimeData.getStringF('cancelButtonText');

    if (args.promptForOpenFile) {
      $('open-button').addEventListener('click', function() {
            var file_path = document.getElementById("file-name-inputbox").value;
            chrome.send('done', [file_path]);
            self.close();
      });
      $('open-button').innerText =
        loadTimeData.getStringF('openButtonText');
      $('save-button').style.display = 'none';
    } else {
      $('save-button').addEventListener('click', function() {
          var file_path = document.getElementById("file-name-inputbox").value;
          chrome.send('done', [file_path]);
          self.close();
      });
      $('save-button').innerText =
        loadTimeData.getStringF('saveButtonText');
      $('open-button').style.display = 'none';
   }

    $('button-row').style['text-align'] = 'end';
  }

  return {
    initialize: initialize
  };
});

document.addEventListener('DOMContentLoaded',
                          filePicker.initialize);

