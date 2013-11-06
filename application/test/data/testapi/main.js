setTimeout(function() {
  xwalk.app.test.notifyFail(function() {
    setTimeout(function() {
      xwalk.app.test.notifyPass(function() {});
    }, 500);
  });
}, 200);
