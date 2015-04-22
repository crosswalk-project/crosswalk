// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

interface XWalkLibraryListener {
    public enum LibraryStatus {
        MATCHED,
        NOT_FOUND,
        SIGNATURE_CHECK_ERROR,
        NEWER_VERSION,
        OLDER_VERSION
    }

    public void onXWalkLibraryCancelled();
    public void onXWalkLibraryMatched();
    public void onXWalkLibraryStartupError(LibraryStatus status, Throwable e);
    public void onXWalkLibraryRuntimeError(LibraryStatus status, Throwable e);

    public void onObjectInitFailed(Object object);
    public void onMethodCallMissed(ReflectMethod method);
}
