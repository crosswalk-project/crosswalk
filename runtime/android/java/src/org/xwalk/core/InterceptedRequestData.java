// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import org.chromium.base.CalledByNative;
import org.chromium.base.JNINamespace;

import java.io.InputStream;

/**
 * The response information that is to be returned for a particular resource fetch.
 */
@JNINamespace("xwalk")
public class InterceptedRequestData {
    private String mMimeType;
    private String mCharset;
    private InputStream mData;

    public InterceptedRequestData(String mimeType, String encoding, InputStream data) {
        mMimeType = mimeType;
        mCharset = encoding;
        mData = data;
    }

    @CalledByNative
    public String getMimeType() {
        return mMimeType;
    }

    @CalledByNative
    public String getCharset() {
        return mCharset;
    }

    @CalledByNative
    public InputStream getData() {
        return mData;
    }
}
