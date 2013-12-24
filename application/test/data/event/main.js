var EventTest = function(resolve) {
  xwalk.app.runtime.onLaunched.addListener(function() {
    resolve();
  });

  xwalk.app.runtime.onInstalled.addListener(function() {
    xwalk.app.test.notifyPass();
  });
};

var tests = [
  EventTest,
];

// Wait at most 10 seconds, see commen in eventapi/main.js
xwalk.app.test.runTests(tests, 10000);
