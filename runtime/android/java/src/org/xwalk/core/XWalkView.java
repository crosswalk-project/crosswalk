// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.Context;
import android.content.Intent;
import android.graphics.Rect;
import android.view.ViewGroup;
import android.util.AttributeSet;
import android.webkit.WebSettings;
import android.widget.FrameLayout;

import org.xwalk.core.client.XWalkDefaultWebChromeClient;
import org.xwalk.core.XWalkDevToolsServer;

public class XWalkView extends FrameLayout {

    XWalkContent mContent;
    XWalkDevToolsServer mDevToolsServer;

    public XWalkView(Context context) {
        this(context, null);
    }

    /**
     * Constructors for inflating via XML.
     */
    public XWalkView(Context context, AttributeSet attrs) {
        super(context, attrs);

        // Intialize library, paks and others.
        XWalkViewDelegate.init(context);

        initXWalkContent(context, attrs);
    }

    private void initXWalkContent(Context context, AttributeSet attrs) {
        mContent = new XWalkContent(context, attrs, this);
        addView(mContent,
                new FrameLayout.LayoutParams(
                        FrameLayout.LayoutParams.MATCH_PARENT,
                        FrameLayout.LayoutParams.MATCH_PARENT));

        // Set default XWalkWebChromeClient.
        setXWalkWebChromeClient(new XWalkDefaultWebChromeClient(context));
    }

    public void loadUrl(String url) {
        mContent.loadUrl(url);
    }

    public void addJavascriptInterface(Object object, String name) {
        mContent.addJavascriptInterface(object, name);
    }

    public String getUrl() {
        return mContent.getUrl();
    }

    public void clearCache(boolean includeDiskFiles) {
        mContent.clearCache(includeDiskFiles);
    }

    public void clearHistory() {
    }

    public boolean canGoBack() {
        return false;
    }

    public void goBack() {
    }

    public boolean canGoForward() {
        return false;
    }

    public void goForward() {
    }

    public boolean requestFocus(int direction, Rect previouslyFocusedRect) {
        return false;
    }

    public void setLayoutParams(ViewGroup.LayoutParams params) {
        super.setLayoutParams(params);
    }

    // TODO(yongsheng): Will replace WebSettings with our class.
    public WebSettings getSettings() {
        return null;
    }

    public void setNetworkAvailable(boolean networkUp) {
    }

    public void setInitialScale(int scaleInPercent) {
    }

    public void setXWalkWebChromeClient(XWalkWebChromeClient client) {
        mContent.setXWalkWebChromeClient(client);
    }

    public void setXWalkClient(XWalkClient client) {
    }

    public void enableRemoteDebugging() {
        if (mDevToolsServer != null) {
            disableRemoteDebugging();
        }
        // Chrome looks for "devtools_remote" pattern in the name of a unix domain socket
        // to identify a debugging page
        final String socketName = getContext().getPackageName();
        mDevToolsServer = new XWalkDevToolsServer(socketName + "_devtools_remote");
        mDevToolsServer.setRemoteDebuggingEnabled(true);
    }

    public void disableRemoteDebugging() {
        if (mDevToolsServer ==  null) {
            return;
        }
        mDevToolsServer.setRemoteDebuggingEnabled(false);
        mDevToolsServer.destroy();
        mDevToolsServer = null;
    }

    public void onPause() {
        mContent.onPause();
    }

    public void onResume() {
        mContent.onResume();
    }

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        mContent.onActivityResult(requestCode, resultCode, data);
    }

    // TODO(shouqun): requestFocusFromTouch, setVerticalScrollBarEnabled are
    // from android.view.View;
}
