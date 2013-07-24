// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import org.chromium.base.ThreadUtils;

class JsResultHandler implements JsResult, JsPromptResult {
    private XWalkContentsClientBridge mBridge;
    private final int mId;

    JsResultHandler(XWalkContentsClientBridge bridge, int id) {
        mBridge = bridge;
        mId = id;
    }

    @Override
    public void confirm() {
        confirm(null);
    }

    @Override
    public void confirm(final String promptResult) {
        ThreadUtils.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mBridge != null)
                    mBridge.confirmJsResult(mId, promptResult);
                mBridge = null;
            }
        });
    }

    @Override
    public void cancel() {
        ThreadUtils.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mBridge != null)
                    mBridge.cancelJsResult(mId);
                mBridge = null;
            }
        });
    }
}
