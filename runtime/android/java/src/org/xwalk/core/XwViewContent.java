// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.Context;
import android.util.AttributeSet;
import android.graphics.Rect;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import org.chromium.base.JNINamespace;
import org.chromium.content.browser.ContentView;
import org.chromium.content.browser.ContentViewCore;
import org.chromium.content.browser.ContentViewRenderView;
import org.chromium.content.browser.LoadUrlParams;
import org.chromium.ui.WindowAndroid;

@JNINamespace("xwalk")
class XwViewContent extends FrameLayout {
    ContentViewCore mContentViewCore;
    ContentView mContentView;
    ContentViewRenderView mContentViewRenderView;
    WindowAndroid mWindow;

    int mXwViewContent;
    int mWebContents;
    boolean mReadyToLoad = false;

    public XwViewContent(Context context, AttributeSet attrs) {
        super(context, attrs);
        //TODO(yongsheng): initialize ContentVideoView

        // initialize ContentViewRenderView
        mContentViewRenderView = new ContentViewRenderView(context) {
            protected void onReadyToRender() {
                mReadyToLoad = true;
            }
        };
        addView(mContentViewRenderView,
                new FrameLayout.LayoutParams(
                        FrameLayout.LayoutParams.MATCH_PARENT,
                        FrameLayout.LayoutParams.MATCH_PARENT));

        //TODO(yongsheng): init native such as resource, content main, etc.

        mXwViewContent = nativeInit();
        mWebContents = nativeGetWebContents(mXwViewContent);

        // initialize ContentView
        mContentView = ContentView.newInstance(
                getContext(), mWebContents, mWindow, ContentView.PERSONALITY_CHROME);
        addView(mContentView,
                new FrameLayout.LayoutParams(
                        FrameLayout.LayoutParams.MATCH_PARENT,
                        FrameLayout.LayoutParams.MATCH_PARENT));

        mContentViewRenderView.setCurrentContentView(mContentView);

        // For addJavascriptInterface
        mContentViewCore = mContentView.getContentViewCore();
    }

    public void loadUrl(String url) {
        if (mReadyToLoad) {
            //TODO(yongsheng): configure appropriate parameters here.
            LoadUrlParams params = new LoadUrlParams(url);
            mContentView.loadUrl(params);
            mContentView.clearFocus();
            mContentView.requestFocus();
        }
    }

    public void addJavascriptInterface(Object object, String name) {
        mContentViewCore.addJavascriptInterface(object, name);
    }

    private native int nativeInit();
    private native int nativeGetWebContents(int nativeXwViewContent);

}
