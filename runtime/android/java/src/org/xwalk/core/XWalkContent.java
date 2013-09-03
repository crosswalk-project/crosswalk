// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Rect;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import org.chromium.base.JNINamespace;
import org.chromium.content.browser.ContentVideoView;
import org.chromium.content.browser.ContentView;
import org.chromium.content.browser.ContentViewCore;
import org.chromium.content.browser.ContentViewRenderView;
import org.chromium.content.browser.LoadUrlParams;
import org.chromium.content.browser.NavigationHistory;
import org.chromium.ui.WindowAndroid;

@JNINamespace("xwalk")
/**
 * This class is the implementation class for XWalkView by calling internal
 * various classes.
 */
public class XWalkContent extends FrameLayout {
    private ContentViewCore mContentViewCore;
    private ContentView mContentView;
    private ContentViewRenderView mContentViewRenderView;
    private WindowAndroid mWindow;
    private XWalkView mXWalkView;
    private XWalkContentsClientBridge mContentsClientBridge;
    private XWalkWebContentsDelegateAdapter mXWalkContentsDelegateAdapter;
    private XWalkSettings mSettings;

    int mXWalkContent;
    int mWebContents;
    boolean mReadyToLoad = false;
    String mPendingUrl = null;

    public XWalkContent(Context context, AttributeSet attrs, XWalkView xwView) {
        super(context, attrs);

        // Initialize the WebContensDelegate.
        mXWalkView = xwView;
        mContentsClientBridge = new XWalkContentsClientBridge(mXWalkView);
        mXWalkContentsDelegateAdapter = new XWalkWebContentsDelegateAdapter(
            mContentsClientBridge);

        // Initialize ContentViewRenderView
        mContentViewRenderView = new ContentViewRenderView(context) {
            protected void onReadyToRender() {
                if (mPendingUrl != null) {
                    doLoadUrl(mPendingUrl);
                    mPendingUrl = null;
                }

                mReadyToLoad = true;
            }
        };
        addView(mContentViewRenderView,
                new FrameLayout.LayoutParams(
                        FrameLayout.LayoutParams.MATCH_PARENT,
                        FrameLayout.LayoutParams.MATCH_PARENT));

        mXWalkContent = nativeInit(mXWalkContentsDelegateAdapter, mContentsClientBridge);
        mWebContents = nativeGetWebContents(mXWalkContent);

        // Initialize mWindow which is needed by content
        mWindow = new WindowAndroid(xwView.getActivity());

        // Initialize ContentView.
        mContentView = ContentView.newInstance(getContext(), mWebContents, mWindow);
        addView(mContentView,
                new FrameLayout.LayoutParams(
                        FrameLayout.LayoutParams.MATCH_PARENT,
                        FrameLayout.LayoutParams.MATCH_PARENT));
        mContentView.setContentViewClient(mContentsClientBridge);

        mContentViewRenderView.setCurrentContentView(mContentView);

        // For addJavascriptInterface
        mContentViewCore = mContentView.getContentViewCore();
        mContentsClientBridge.installWebContentsObserver(mContentViewCore);

        mSettings = new XWalkSettings(getContext(), mWebContents, true);
    }

    void doLoadUrl(String url) {
        //TODO(Xingnan): Configure appropriate parameters here.
        // Handle the same url loading by parameters.
        if (TextUtils.equals(url, mContentView.getUrl())) {
            mContentView.reload();
        } else {
            mContentView.loadUrl(new LoadUrlParams(url));
        }

        mContentView.clearFocus();
        mContentView.requestFocus();
    }

    public void loadUrl(String url) {
        if (url == null)
            return;

        if (mReadyToLoad)
            doLoadUrl(url);
        else
            mPendingUrl = url;
    }

    public String getUrl() {
        String url = mContentView.getUrl();
        if (url == null || url.trim().isEmpty()) return null;
        return url;
    }

    public void addJavascriptInterface(Object object, String name) {
        mContentViewCore.addJavascriptInterface(object, name);
    }

    public void setXWalkWebChromeClient(XWalkWebChromeClient client) {
        mContentsClientBridge.setXWalkWebChromeClient(client);
    }

    public void setXWalkClient(XWalkClient client) {
        mContentsClientBridge.setXWalkClient(client);
    }

    public void onPause() {
        mContentViewCore.onActivityPause();
    }

    public void onResume() {
        mContentViewCore.onActivityResume();
    }

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        mWindow.onActivityResult(requestCode, resultCode, data);
    }

    public void clearCache(boolean includeDiskFiles) {
        if (mXWalkContent == 0) return;
        nativeClearCache(mXWalkContent, includeDiskFiles);
    }

    public boolean canGoBack() {
        return mContentView.canGoBack();
    }

    public void goBack() {
        mContentView.goBack();
    }

    public boolean canGoForward() {
        return mContentView.canGoForward();
    }

    public void goForward() {
        mContentView.goForward();
    }

    public void stopLoading() {
        mContentView.stopLoading();
    }

    public String getOriginalUrl() {
        NavigationHistory history = mContentViewCore.getNavigationHistory();
        int currentIndex = history.getCurrentEntryIndex();
        if (currentIndex >= 0 && currentIndex < history.getEntryCount()) {
            return history.getEntryAtIndex(currentIndex).getOriginalUrl();
        }
        return null;
    }

    // For instrumentation test.
    public ContentViewCore getContentViewCoreForTest() {
        return mContentViewCore;
    }

    // For instrumentation test.
    public void installWebContentsObserverForTest(XWalkContentsClient contentClient) {
        contentClient.installWebContentsObserver(mContentViewCore);
    }

    public String devToolsAgentId() {
        return nativeDevToolsAgentId(mXWalkContent);
    }

    public XWalkSettings getSettings() {
        return mSettings;
    }

    private native int nativeInit(XWalkWebContentsDelegate webViewContentsDelegate,
            XWalkContentsClientBridge bridge);

    private native int nativeGetWebContents(int nativeXWalkContent);
    private native void nativeClearCache(int nativeXWalkContent, boolean includeDiskFiles);
    private native String nativeDevToolsAgentId(int nativeXWalkContent);
}
