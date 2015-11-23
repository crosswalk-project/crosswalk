// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;

/**
 * It's for http auth handling.
 * @hide
 */
@JNINamespace("xwalk")
@XWalkAPI(createExternally = true)
public class XWalkHttpAuthHandlerInternal {

    private long mNativeXWalkHttpAuthHandler;
    private final boolean mFirstAttempt;

    @XWalkAPI
    public void proceed(String username, String password) {
        if (mNativeXWalkHttpAuthHandler != 0) {
            nativeProceed(mNativeXWalkHttpAuthHandler, username, password);
            mNativeXWalkHttpAuthHandler = 0;
        }
    }

    @XWalkAPI
    public void cancel() {
        if (mNativeXWalkHttpAuthHandler != 0) {
            nativeCancel(mNativeXWalkHttpAuthHandler);
            mNativeXWalkHttpAuthHandler = 0;
        }
    }

    public boolean isFirstAttempt() {
         return mFirstAttempt;
    }

    @CalledByNative
    public static XWalkHttpAuthHandlerInternal create(long nativeXWalkAuthHandler, boolean firstAttempt) {
        return new XWalkHttpAuthHandlerInternal(nativeXWalkAuthHandler, firstAttempt);
    }

    @XWalkAPI
    public XWalkHttpAuthHandlerInternal(long nativeXWalkHttpAuthHandler, boolean firstAttempt) {
        mNativeXWalkHttpAuthHandler = nativeXWalkHttpAuthHandler;
        mFirstAttempt = firstAttempt;
    }

    @CalledByNative
    void handlerDestroyed() {
        mNativeXWalkHttpAuthHandler = 0;
    }

    private native void nativeProceed(long nativeXWalkHttpAuthHandler,
            String username, String password);
    private native void nativeCancel(long nativeXWalkHttpAuthHandler);
}

