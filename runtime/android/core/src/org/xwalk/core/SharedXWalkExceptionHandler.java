// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

public abstract class SharedXWalkExceptionHandler {
    boolean handleException(Throwable e) {
        onSharedLibraryNotFound();
        return true;
    }

    public abstract void onSharedLibraryNotFound();
}
