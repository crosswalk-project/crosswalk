// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import org.chromium.base.CalledByNative;
import org.chromium.base.JNINamespace;

/**
 * Delegate for handling callbacks. All methods are called on the IO thread.
 */
@JNINamespace("xwalk")
interface XWalkContentsIoThreadClient {
    @CalledByNative
    public int getCacheMode();

    @CalledByNative
    public InterceptedRequestData shouldInterceptRequest(String url, boolean isMainFrame);

    @CalledByNative
    public boolean shouldBlockContentUrls();

    @CalledByNative
    public boolean shouldBlockFileUrls();

    @CalledByNative
    public boolean shouldBlockNetworkLoads();

    @CalledByNative
    public void onDownloadStart(String url,
                                String userAgent,
                                String contentDisposition,
                                String mimeType,
                                long contentLength);

    @CalledByNative
    public void newLoginRequest(String realm, String account, String args);
}
