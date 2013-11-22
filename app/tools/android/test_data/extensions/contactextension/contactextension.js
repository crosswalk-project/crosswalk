// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var readCallback = null;
extension.setMessageListener(function(phone) {
    if (readCallback instanceof Function) {
        readCallback(phone);
    };
});
// Export API 'read'.
exports.read = function(name, callback) {
    readCallback = callback;
    extension.postMessage(JSON.stringify({"cmd":"read", "name":name}));
};
// Export API 'write'.
exports.write = function(name, phone) {
    extension.postMessage(JSON.stringify(
                {"cmd": "write",
                 "name": name,
                 "phone": phone
                }));
};
