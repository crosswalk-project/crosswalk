// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.Context;
import android.graphics.Rect;
import android.view.ViewGroup;
import android.util.AttributeSet;
import android.webkit.WebChromeClient;
import android.webkit.WebSettings;
import android.webkit.WebViewClient;
import android.widget.FrameLayout;

public class XwView extends FrameLayout {

    XwViewContent mContent;

    public XwView(Context context) {
        this(context, null);
    }

    /**
     * Constructors for inflating via XML.
     */
    public XwView(Context context, AttributeSet attrs) {
        super(context, attrs);

        // intialize library, paks and others
        XwViewDelegate.init(context);

        // intialize XwViewContent
        initXwViewContent(context, attrs);

    }

    private void initXwViewContent(Context context, AttributeSet attrs) {
        mContent = new XwViewContent(context, attrs);
        addView(mContent,
                new FrameLayout.LayoutParams(
                        FrameLayout.LayoutParams.MATCH_PARENT,
                        FrameLayout.LayoutParams.MATCH_PARENT));
    }

    public void loadUrl(String url) {
        mContent.loadUrl(url);
    }

    public void addJavascriptInterface(Object object, String name) {
        mContent.addJavascriptInterface(object, name);
    }

    public String getUrl() {
        return "";
    }

    public void clearCache(boolean includeDiskFiles) {
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

    public WebSettings getSettings() {
        return null;
    }

    public void setNetworkAvailable(boolean networkUp) {
    }

    public void setInitialScale(int scaleInPercent) {
    }

    public void setWebChromeClient(WebChromeClient client) {
    }

    public void setWebViewClient(WebViewClient client) {
    }

    //requestFocusFromTouch, setVerticalScrollBarEnabled are from android.view.View;
}
