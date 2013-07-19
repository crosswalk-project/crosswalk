// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package com.intel.xwalk;

import android.content.Context;
import android.util.AttributeSet;
import android.webkit.WebView;
import android.widget.FrameLayout;

import org.chromium.base.CalledByNative;
import org.chromium.base.JNINamespace;
import org.chromium.content.browser.ContentView;
import org.chromium.content.browser.ContentViewRenderView;
import org.chromium.content.browser.LoadUrlParams;
import org.chromium.ui.WindowAndroid;

@JNINamespace("xwalk")
public class XWalkView extends FrameLayout {
   private WindowAndroid mWindowAndroid;
   private ContentView mContentView;
   private ContentViewRenderView mContentViewRenderView; // SurfaceView

   public XWalkView(Context context, AttributeSet attrs) {
        super(context, attrs);
        nativeInit(this);

        mContentViewRenderView = new ContentViewRenderView(context) {
            @Override
            protected void onReadyToRender() {
                mContentView.loadUrl(new LoadUrlParams("http://www.google.com"));
                mContentView.clearFocus();
                mContentView.requestFocus();
            }
        };
    }

    public void setWindow(WindowAndroid window) {
        mWindowAndroid = window;
    }

    public void loadUrl(String url) {
    }

    @SuppressWarnings("unused")
    @CalledByNative
    private void initWithWebContents(int webContents) {
        addView(mContentViewRenderView, new FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.MATCH_PARENT,
            FrameLayout.LayoutParams.MATCH_PARENT));

        mContentView = ContentView.newInstance(getContext(), webContents,
            mWindowAndroid, ContentView.PERSONALITY_VIEW);

        addView(mContentView, new FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.MATCH_PARENT,
            FrameLayout.LayoutParams.MATCH_PARENT));

        mContentViewRenderView.setCurrentContentView(mContentView);
    }

    private static native void nativeInit(Object view);
}

