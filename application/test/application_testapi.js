var internal = requireNative("internal");
internal.setupInternalExtension(extension);

exports.notifyPass = function(callback) {
  internal.postMessage('notifyPass', [], callback);
}

exports.notifyFail = function(callback) {
  internal.postMessage('notifyFail', [], callback);
}
