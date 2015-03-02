// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.app.Activity;
import android.content.Context;
import android.view.View;
import android.view.WindowManager;

import org.chromium.base.CommandLine;
import org.chromium.content.browser.ContentVideoView;
import org.chromium.content.browser.ContentVideoViewClient;
import org.chromium.content.common.ContentSwitches;
import org.xwalk.core.internal.XWalkWebChromeClient.CustomViewCallback;

class XWalkContentVideoViewClient implements ContentVideoViewClient {
    private XWalkContentsClient mContentsClient;
    private Activity mActivity;
    private XWalkViewInternal mView;

    public XWalkContentVideoViewClient(XWalkContentsClient client, Activity activity, XWalkViewInternal view) {
        mContentsClient = client;
        mActivity = activity;
        mView = view;
    }

    @Override
    public void enterFullscreenVideo(View view) {
        mView.setOverlayVideoMode(true);
        CustomViewCallback cb = new CustomViewCallback() {
            @Override
            public void onCustomViewHidden() {
                ContentVideoView contentVideoView = ContentVideoView.getContentVideoView();
                if (contentVideoView != null) contentVideoView.exitFullscreen(false);
            }
        };
        mContentsClient.onShowCustomView(view, cb);
    }

    @Override
    public void exitFullscreenVideo() {
        mView.setOverlayVideoMode(false);
        mContentsClient.onHideCustomView();
    }

    @Override
    public View getVideoLoadingProgressView() {
        return null;
    }
    @Override
    public void setSystemUiVisibility(boolean enterFullscreen) {
    }
}
