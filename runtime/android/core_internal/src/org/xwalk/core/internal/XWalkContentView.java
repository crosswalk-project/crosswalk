// Copyright 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.content.Context;
import android.graphics.Rect;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.accessibility.AccessibilityNodeProvider;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewStructure;

import org.chromium.content.browser.ContentView;
import org.chromium.content.browser.ContentViewCore;

public class XWalkContentView extends ContentView {
    private static final String TAG = "XWalkContentView";
    private XWalkViewInternal mXWalkView;

    public static XWalkContentView createContentView(Context context, ContentViewCore cvc,
            XWalkViewInternal xwView) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            return new XWalkContentViewApi23(context, cvc, xwView);
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
            return new XWalkContentViewApi16(context, cvc, xwView);
        }
        return new XWalkContentView(context, cvc, xwView);
    }

    private XWalkContentView(Context context, ContentViewCore cvc, XWalkViewInternal xwView) {
        super(context, cvc);
        mXWalkView = xwView;
    }

    @Override
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        return mXWalkView.onCreateInputConnection(outAttrs);
    }

    public InputConnection onCreateInputConnectionSuper(EditorInfo outAttrs) {
        return super.onCreateInputConnection(outAttrs);
    }

    @Override
    public boolean performLongClick(){
        return mXWalkView.performLongClickDelegate();
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        // Give XWalkView a chance to handle touch event
        if(mXWalkView.onTouchEventDelegate(event)) {
            return true;
        }
        return mContentViewCore.onTouchEvent(event);
    }

    @Override
    public void onScrollChanged(int l, int t, int oldl, int oldt) {
        mXWalkView.onScrollChangedDelegate(l, t, oldl, oldt);

        // To keep the same behaviour with WebView onOverScrolled API,
        // call onOverScrolledDelegate here.
        mXWalkView.onOverScrolledDelegate(l, t, false, false);
    }

    /**
     * Since compute* APIs in ContentView are all protected, use delegate methods
     * to get the result.
     */
    public int computeHorizontalScrollRangeDelegate() {
        return computeHorizontalScrollRange();
    }

    public int computeHorizontalScrollOffsetDelegate() {
        return computeHorizontalScrollOffset();
    }

    public int computeVerticalScrollRangeDelegate() {
        return computeVerticalScrollRange();
    }

    public int computeVerticalScrollOffsetDelegate() {
        return computeVerticalScrollOffset();
    }

    public int computeVerticalScrollExtentDelegate() {
        return computeVerticalScrollExtent();
    }

    @Override
    protected void onFocusChanged(boolean gainFocus, int direction, Rect previouslyFocusedRect) {
        mXWalkView.onFocusChangedDelegate(gainFocus, direction, previouslyFocusedRect);
        super.onFocusChanged(gainFocus, direction, previouslyFocusedRect);
    }

    // Imitate JellyBeanContentView
    private static class XWalkContentViewApi16 extends XWalkContentView {
        public XWalkContentViewApi16(Context context, ContentViewCore cvc,
                XWalkViewInternal xwView) {
            super(context, cvc, xwView);
        }

        @Override
        public boolean performAccessibilityAction(int action, Bundle arguments) {
            if (mContentViewCore.supportsAccessibilityAction(action)) {
                return mContentViewCore.performAccessibilityAction(action, arguments);
            }

            return super.performAccessibilityAction(action, arguments);
        }

        // Copy the implementation of JellyBeanContentView
        @Override
        public AccessibilityNodeProvider getAccessibilityNodeProvider() {
            AccessibilityNodeProvider provider = mContentViewCore.getAccessibilityNodeProvider();
            if (provider != null) {
                return provider;
            } else {
                return super.getAccessibilityNodeProvider();
            }
        }
    }

    // Imitate ContentView.ContentViewApi23
    private static class XWalkContentViewApi23 extends XWalkContentViewApi16 {
        public XWalkContentViewApi23(Context context, ContentViewCore cvc,
                XWalkViewInternal xwView) {
            super(context, cvc, xwView);
        }

        @Override
        public void onProvideVirtualStructure(final ViewStructure structure) {
            mContentViewCore.onProvideVirtualStructure(structure, false);
        }
    }
}
