var notifyFailTest = function(resolve) {
  setTimeout(function() {
    xwalk.app.test.notifyFail(function() {
      resolve();
    });
  }, 100);
};

var notifyPassTest = function(resolve) {
  setTimeout(function() {
    xwalk.app.test.notifyPass(function() {
      resolve();
    });
  }, 100);
};

var notifyTimeoutTest = function(resolve) {
  setTimeout(function() {
    xwalk.app.test.notifyTimeout(function() {
      resolve();
    });
  }, 100);
};

var tests = [
  notifyFailTest,
  notifyPassTest,
  notifyTimeoutTest,
];

xwalk.app.test.runTests(tests);
