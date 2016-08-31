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
import org.chromium.content.browser.ContentVideoViewEmbedder;
import org.chromium.content.common.ContentSwitches;

class XWalkContentVideoViewClient implements ContentVideoViewEmbedder {
    private XWalkContentsClient mContentsClient;
    private XWalkViewInternal mView;

    public XWalkContentVideoViewClient(XWalkContentsClient client, XWalkViewInternal view) {
        mContentsClient = client;
        mView = view;
    }

    @Override
    public void enterFullscreenVideo(View view) {
        mView.setOverlayVideoMode(true);
        mContentsClient.onShowCustomView(view, new CustomViewCallbackHandlerInternal());
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
