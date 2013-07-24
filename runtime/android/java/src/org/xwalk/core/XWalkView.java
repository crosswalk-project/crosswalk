// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.Context;
import android.graphics.Rect;
import android.view.ViewGroup;
import android.util.AttributeSet;
import android.webkit.WebSettings;
import android.widget.FrameLayout;

public class XWalkView extends FrameLayout {

    XWalkContent mContent;

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

    // TODO(yongsheng): Will replace WebSettings with our class.
    public WebSettings getSettings() {
        return null;
    }

    public void setNetworkAvailable(boolean networkUp) {
    }

    public void setInitialScale(int scaleInPercent) {
    }

    public void setXWalkWebChromeClient(XWalkWebChromeClient client) {
    }

    public void setXWalkClient(XWalkClient client) {
    }

    // TODO(shouqun): requestFocusFromTouch, setVerticalScrollBarEnabled are
    // from android.view.View;
}
