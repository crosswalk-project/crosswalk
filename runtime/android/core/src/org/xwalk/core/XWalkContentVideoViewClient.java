// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.content.Context;
import android.view.View;
import android.view.WindowManager;

import org.chromium.base.CommandLine;
import org.chromium.content.browser.ContentVideoViewClient;
import org.chromium.content.common.ContentSwitches;
import org.xwalk.core.XWalkWebChromeClient.CustomViewCallback;

class XWalkContentVideoViewClient implements ContentVideoViewClient {
    private XWalkContentsClient mContentsClient;
    private Activity mActivity;
    private XWalkView mView;

    public XWalkContentVideoViewClient(XWalkContentsClient client, Activity activity, XWalkView view) {
        mContentsClient = client;
        mActivity = activity;
        mView = view;
    }

    @Override
    public boolean onShowCustomView(View view) {
        if (!CommandLine.getInstance().hasSwitch(
                ContentSwitches.DISABLE_OVERLAY_FULLSCREEN_VIDEO_SUBTITLE)) {
            mView.setOverlayVideoMode(true);
        }

        CustomViewCallback cb = new CustomViewCallback() {
            @Override
            public void onCustomViewHidden() {
            }
        };
        mContentsClient.onShowCustomView(view, cb);
        return true;
    }

    @Override
    public void onDestroyContentVideoView() {
        if (!CommandLine.getInstance().hasSwitch(
                ContentSwitches.DISABLE_OVERLAY_FULLSCREEN_VIDEO_SUBTITLE)) {
            mView.setOverlayVideoMode(false);
        }
        mContentsClient.onHideCustomView();
    }

    @Override
    public View getVideoLoadingProgressView() {
        return mContentsClient.getVideoLoadingProgressView();
    }
}
