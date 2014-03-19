// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import org.chromium.base.ThreadUtils;

class XWalkJavascriptResultHandler implements XWalkJavascriptResult {
    private XWalkContentsClientBridge mBridge;
    private final int mId;

    XWalkJavascriptResultHandler(XWalkContentsClientBridge bridge, int id) {
        mBridge = bridge;
        mId = id;
    }

    @Override
    public void confirm() {
        confirmWithResult(null);
    }

    @Override
    public void confirmWithResult(final String promptResult) {
        ThreadUtils.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mBridge != null) {
                    mBridge.confirmJsResult(mId, promptResult);
                }
                mBridge = null;
            }
        });
    }

    @Override
    public void cancel() {
        ThreadUtils.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mBridge != null) {
                    mBridge.cancelJsResult(mId);
                }
                mBridge = null;
            }
        });
    }
}
