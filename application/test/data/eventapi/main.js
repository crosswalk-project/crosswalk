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

function dummy(arg1, arg2) {
  runTestCase(function() {
    assert(arg1 === 1234);
    assert(arg2 === false);
    dummy_called = true;
  });
};

function bad() {
  assert(false); 
};

function foo(arg1, arg2) {
  runTestCase(function() {
    assert(dummy_called === true);
    assert(arg1 === 1234);
    assert(arg2 === false);
    xwalk.app.test.notifyPass(function(){});
  });
};

runTestCase(function() {
  onMockEvent = new xwalk.app.events.Event("onMockEvent");
  var onMockEvent2 = new xwalk.app.events.Event("onMockEvent");

  // Add/remove listener.
  onMockEvent.addListener(dummy);
  assert(onMockEvent.hasListener(dummy));
  assert(onMockEvent.hasListeners());
  onMockEvent.removeListener(dummy);
  assert(!onMockEvent.hasListeners());

  // Register a named event twice should fail.
  onMockEvent.addListener(dummy);
  try {
    onMockEvent2.addListener(dummy);
    assert(false);
  } catch (e) {
    assert(e.message.search("already registered") >= 0);
  }

  xwalk.app.test.notifyPass(function(){});
});

// The "bad" handler will triggers exception which catched by Event.dispatchEvent.
onMockEvent.addListener(bad);
onMockEvent.addListener(foo);

// Wait one sec, if the foo listener is not trigger then return fail.
setTimeout(function() {
  xwalk.app.test.notifyFail(function() {});
}, 1000);
