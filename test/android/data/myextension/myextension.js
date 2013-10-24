var echoListener = null;
extension.setMessageListener(function(msg) {
  if (echoListener instanceof Function) {
    echoListener(msg);
  };
});
// Export API 'echo'.
exports.echo = function(msg, callback) {
  echoListener = callback;
  extension.postMessage(msg);
};
// Export API 'echoSync'.
exports.echoSync = function(msg) {
  return extension.internal.sendSyncMessage(msg);
};
