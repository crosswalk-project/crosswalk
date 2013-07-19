// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.content.Context;
import android.util.AttributeSet;
import android.graphics.Rect;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import org.chromium.base.JNINamespace;
import org.chromium.content.browser.ContentVideoView;
import org.chromium.content.browser.ContentView;
import org.chromium.content.browser.ContentViewCore;
import org.chromium.content.browser.ContentViewRenderView;
import org.chromium.content.browser.LoadUrlParams;
import org.chromium.ui.WindowAndroid;

@JNINamespace("xwalk")
/**
 * This class is the implementation class for XwView by calling internal
 * various classes.
 */
class XwViewContent extends FrameLayout {
    private ContentViewCore mContentViewCore;
    private ContentView mContentView;
    private ContentViewRenderView mContentViewRenderView;
    private WindowAndroid mWindow;
    private XwView mXwView;
    private XwContentsClient mContentsClient;
    private XwContentsClientBridge mContentsClientBridge;
    private XwWebContentsDelegateAdapter mXwContentsDelegateAdapter;

    int mXwViewContent;
    int mWebContents;
    boolean mReadyToLoad = false;

    public XwViewContent(Context context, AttributeSet attrs, XwView xwView) {
        super(context, attrs);

        // Initialize the WebContensDelegate.
        mXwView = xwView;
        mContentsClientBridge = new XwContentsClientBridge(mXwView);
        mXwContentsDelegateAdapter = new XwWebContentsDelegateAdapter(
            mContentsClientBridge);

        // Initialize ContentViewRenderView
        mContentViewRenderView = new ContentViewRenderView(context) {
            protected void onReadyToRender() {
                mReadyToLoad = true;
            }
        };
        addView(mContentViewRenderView,
                new FrameLayout.LayoutParams(
                        FrameLayout.LayoutParams.MATCH_PARENT,
                        FrameLayout.LayoutParams.MATCH_PARENT));

        mXwViewContent = nativeInit();
        mWebContents = nativeGetWebContents(mXwViewContent);

        // Initialize mWindow which is needed by content
        if (getContext() instanceof Activity) {
            Activity activity = (Activity) getContext();
            mWindow = new WindowAndroid(activity);
        }

        // Initialize the ContentVideoView for fullscreen video playback.
        ContentVideoView.registerContentVideoViewContextDelegate(
                new XwContentVideoViewDelegate(mContentsClientBridge, getContext()));

        // Initialize ContentView
        // TODO(yongsheng): Use PERSONALITY_VIEW if we don't need pinch to zoom.
        // PERSONALITY_VIEW always overrides the user agent set by user.
        mContentView = ContentView.newInstance(
                getContext(), mWebContents, mWindow, ContentView.PERSONALITY_CHROME);
        addView(mContentView,
                new FrameLayout.LayoutParams(
                        FrameLayout.LayoutParams.MATCH_PARENT,
                        FrameLayout.LayoutParams.MATCH_PARENT));

        mContentViewRenderView.setCurrentContentView(mContentView);

        // For addJavascriptInterface
        mContentViewCore = mContentView.getContentViewCore();
        mContentViewCore.setContentViewClient(mContentsClientBridge);
        mContentsClientBridge.installWebContentsObserver(mContentViewCore);

        // TODO(yongsheng): Initialize settings, InterceptNavigationDelegateImpl, IoThreadClientImpl,
        // DIPScale, etc.

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
