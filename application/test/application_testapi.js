// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var internal = requireNative("internal");
internal.setupInternalExtension(extension);

function assert(condition, message) {
  if (!condition)
    throw message || "Assertion failed";
};

// If all tests are not finished before 'maxWaitingTime' milliseconds, then the
// TIMEOUT notification will be sent.
function runTests(tests, maxWaitingTime) {
  var idx = 0;
  function runTestCase(func) {
    return new Promise(function(resolve) {
      func.call(null, resolve);
      }).then(function() {
        idx++;
        if (idx == tests.length) {
          xwalk.app.test.notifyPass();
          return;
        }
        runTestCase(tests[idx]);
      }, function(e) {
        console.error(e);
        xwalk.app.test.notifyFail(); 
      });
  };

  if (typeof(maxWaitingTime) === 'number' && isFinite(maxWaitingTime)) {
    setTimeout(function() {
      xwalk.app.test.notifyTimeout();
      }, maxWaitingTime);
  }

  runTestCase(tests[idx]);
};

exports.notifyPass = function(callback) {
  internal.postMessage('notifyPass', [], callback);
};

exports.notifyFail = function(callback) {
  internal.postMessage('notifyFail', [], callback);
};

exports.notifyTimeout = function(callback) {
  internal.postMessage('notifyTimeout', [], callback);
};

exports.assert = assert;
exports.runTests = runTests;
