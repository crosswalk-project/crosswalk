// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import org.chromium.base.CalledByNative;
import org.chromium.base.JNINamespace;

@JNINamespace("xwalk")
public class XWalkHttpAuthHandler {

    private int mNativeHttpAuthHandler;
    private final boolean mFirstAttempt;

    public void proceed(String username, String password) {
        if (mNativeHttpAuthHandler != 0) {
            nativeProceed(mNativeHttpAuthHandler, username, password);
            mNativeHttpAuthHandler = 0;
        }
    }

    public void cancel() {
        if (mNativeHttpAuthHandler != 0) {
            nativeCancel(mNativeHttpAuthHandler);
            mNativeHttpAuthHandler = 0;
        }
    }

    public boolean isFirstAttempt() {
         return mFirstAttempt;
    }

    @CalledByNative
    public static XWalkHttpAuthHandler create(int nativeAuthHandler, boolean firstAttempt) {
        return new XWalkHttpAuthHandler(nativeAuthHandler, firstAttempt);
    }

    private XWalkHttpAuthHandler(int nativeHttpAuthHandler, boolean firstAttempt) {
        mNativeHttpAuthHandler = nativeHttpAuthHandler;
        mFirstAttempt = firstAttempt;
    }

    @CalledByNative
    void handlerDestroyed() {
        mNativeHttpAuthHandler = 0;
    }

    private native void nativeProceed(int nativeHttpAuthHandler,
            String username, String password);
    private native void nativeCancel(int nativeHttpAuthHandler);
}

