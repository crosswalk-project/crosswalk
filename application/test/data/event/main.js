function assert(condition, message) {
  if (!condition)
    throw message || "Assertion failed";
};

function runTestCase(fn) {
  try {
    fn();
  } catch(e) {
    console.log(e);
    console.log(new Error().stack);
    xwalk.app.test.notifyFail(function(){});
  }
};

runTestCase(function() {
  xwalk.app.runtime.onLaunched.addListener(function() {
    window.open("index.html");
  });
});
