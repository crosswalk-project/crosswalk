// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * Asserts the value is true.
 * @param {*} x The value to assert.
 */
function assert(x) {
  if (!x)
    throw new Error();
}

/**
 * Asserts the values are equal.
 * @param {*} x The value to assert.
 */
function assertEquals(x, y) {
  if (x != y)
    throw new Error(x + ' != ' + y);
}

/**
 * Runs the given test.
 * @param {function()} test The test to run.
 * @param {function()} onPass The function to call if and when the
 *     function passes.
 */
function runTest(test, onPass) {
  var shouldContinue = true;
  var runner = {
    waitForAsync: function(description) {
      shouldContinue = false;
      console.log('Waiting for ', description);
    },

    continueTesting: function() {
      shouldContinue = true;
      window.setTimeout(function() {
        if (shouldContinue)
          onPass();
      }, 0);
    }
  };

  test(runner);
  if (shouldContinue)
    onPass();
}

/**
 * Runs all tests and reports the results via the console.
 */
function runTests() {
  var tests = [];
  for (var i in window) {
    if (i.indexOf('test') == 0)
      tests.push(window[i]);
  }
  console.log('Running %d tests...', tests.length);

  var testNo = 0;
  function runNextTest() {
    if (testNo >= tests.length) {
      console.log('All tests passed');
      return;
    }

    function onPass() {
      testNo++;
      runNextTest();
    }

    var test = tests[testNo];
    console.log('Running (%d/%d) -- %s', testNo + 1, tests.length, test.name);
    runTest(test, onPass);
  }
  runNextTest();
}

window.addEventListener('load', function() {
  runTests();
});
