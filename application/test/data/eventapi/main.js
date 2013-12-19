var assert = xwalk.app.test.assert;
var dummy_called = false;
var eventListenerTest = function(resolve) {
  function dummy(arg1, arg2) {
    dummy_called = true;
  };

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

  resolve();
};

var eventDispatchTest = function(resolve) {
  function bad() {
    assert(false); 
  };

  function foo(arg1, arg2) {
    assert(dummy_called === true);
    assert(arg1 === 1234);
    assert(arg2 === false);
    resolve();
  };

  // The "bad" handler will triggers exception which catched by
  // Event.dispatchEvent.
  onMockEvent.addListener(bad);
  onMockEvent.addListener(foo);

  // Send a notification here to make sure the event listeners are set before
  // event sending.
  xwalk.app.test.notifyPass();
};

var tests = [
  eventListenerTest,
  eventDispatchTest,
];

// Wait at most 10 seconds before sending TIMEOUT fail. When multiple browser
// tests running parallelly the finish time may longer than expected.
xwalk.app.test.runTests(tests, 10000);
