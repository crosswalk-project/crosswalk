// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.net.Uri;

import java.util.Map;

@XWalkAPI(impl = WebResourceRequestInternal.class, createInternally = true)
public class WebResourceRequestHandlerInternal implements WebResourceRequestInternal {
    private final XWalkContentsClientBridge.XWalkWebResourceRequest mRequest;

    WebResourceRequestHandlerInternal(XWalkContentsClientBridge.XWalkWebResourceRequest request) {
        mRequest = request;
    }

    // Never use this constructor.
    // It is only used in WebResourceRequestHandlerBridge.
    WebResourceRequestHandlerInternal() {
        mRequest = null;
    }

    @XWalkAPI
    public Uri getUrl() {
        return Uri.parse(mRequest.url);
    }

    @XWalkAPI
    public boolean isForMainFrame() {
        return mRequest.isMainFrame;
    }

    @XWalkAPI
    public boolean hasGesture() {
        return mRequest.hasUserGesture;
    }

    @XWalkAPI
    public String getMethod() {
        return mRequest.method;
    }

    @XWalkAPI
    public Map<String, String> getRequestHeaders() {
        return mRequest.requestHeaders;
    }
}
