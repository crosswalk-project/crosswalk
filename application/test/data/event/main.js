var tests = [
  function(resolve) {
    xwalk.app.runtime.onLaunched.addListener(function() {
        resolve();
    });
  },
];

// Wait at most 10 seconds, see commen in eventapi/main.js
xwalk.app.test.runTests(tests, 10000);
