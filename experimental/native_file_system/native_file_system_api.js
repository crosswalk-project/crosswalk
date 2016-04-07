// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

let isolated_fs = requireNative("isolated_file_system");
let internal = requireNative("internal");
internal.setupInternalExtension(extension);

class NativeFileSystem {
  // The extension side of the implementation is ugly, this function is not
  // well-thought and the whole thing is needlessly hard to test. At the very
  // least it should be made asynchronous, at best it should be removed
  // altogether.
  // It is being kept for the time being for backwards compatibility.
  getRealPath(virtual_root) {
    return extension.internal.sendSyncMessage({
      'command': 'getRealPath',
      'path': virtual_root
    });
  }

  requestNativeFileSystem(path, success, error) {
    if (typeof path !== "string" || !(success instanceof Function)) {
      throw new TypeError("Wrong parameters passed to requestNativeFileSystem.");
    }
    error = error || (_ => {});

    internal.postMessage(
      "requestNativeFileSystem", [path],
      (filesystem_id, error_message) => {
        if (error_message)
          error(new Error(error_message));
        else
          success(isolated_fs.getIsolatedFileSystem(filesystem_id));
      });
  }
}

exports = new NativeFileSystem();
