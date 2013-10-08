// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.content.Context;
import android.view.View;
import android.view.WindowManager;

import org.chromium.content.browser.ContentVideoViewClient;
import org.chromium.content.browser.ContentVideoViewControls;
import org.xwalk.core.XWalkWebChromeClient.CustomViewCallback;

public class XWalkContentVideoViewClient implements ContentVideoViewClient {
    private XWalkContentsClient mContentsClient;
    private Activity mActivity;

    public XWalkContentVideoViewClient(XWalkContentsClient client, Activity activity) {
        mContentsClient = client;
        mActivity = activity;
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
    public View getVideoLoadingProgressView() {
        return mContentsClient.getVideoLoadingProgressView();
    }

    @Override
    public ContentVideoViewControls createControls() {
        return null;
    }
}
