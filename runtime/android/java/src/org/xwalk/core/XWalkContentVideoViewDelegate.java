// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.Context;
import android.view.View;

import org.chromium.content.browser.ContentVideoViewContextDelegate;
import org.xwalk.core.XWalkWebChromeClient.CustomViewCallback;

public class XWalkContentVideoViewDelegate implements ContentVideoViewContextDelegate {
    private Context mContext;
    private XWalkContentsClient mContentsClient;

    public XWalkContentVideoViewDelegate(XWalkContentsClient client, Context context) {
        mContext = context;
        mContentsClient = client;
    }

    @Override
    public void onShowCustomView(View view) {
        CustomViewCallback cb = new CustomViewCallback() {
            @Override
            public void onCustomViewHidden() {
            }
        };
        mContentsClient.onShowCustomView(view, cb);
    }

    @Override
    public void onDestroyContentVideoView() {
        mContentsClient.onHideCustomView();
    }

    @Override
    public Context getContext() {
        return mContext;
    }

    @Override
    public View getVideoLoadingProgressView() {
        return mContentsClient.getVideoLoadingProgressView();
    }
}
