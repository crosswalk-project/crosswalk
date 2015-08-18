// Copyright 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.content.Context;
import android.os.Build;
import android.os.Bundle;
import android.view.accessibility.AccessibilityNodeProvider;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;

import org.chromium.content.browser.ContentView;
import org.chromium.content.browser.ContentViewCore;

public class XWalkContentView extends ContentView {
    private XWalkViewInternal mXWalkView;

    XWalkContentView(Context context, ContentViewCore cvc, XWalkViewInternal xwView) {
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
    public boolean performAccessibilityAction(int action, Bundle arguments) {
        // Originally, we obtain a ContentView instance through ContentView.newInstance().
        // The method newInstance will return ContentView or JellyBeanContentView
        // respectively according to the sdk version like below:
        // public static ContentView newInstance(Context context, ContentViewCore cvc) {
        //     if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN) {
        //         return new ContentView(context, cvc);
        //     } else {
        //         return new JellyBeanContentView(context, cvc);
        //     }
        // }
        // Now we use XWalkContentView uniformly, so this is a substitute for it.
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN) {
            return super.performAccessibilityAction(action, arguments);
        }

        // Copy code from JellyBeanContentView because the class is not public
        if (mContentViewCore.supportsAccessibilityAction(action)) {
            return mContentViewCore.performAccessibilityAction(action, arguments);
        }

        return super.performAccessibilityAction(action, arguments);
    }

    @Override
    public AccessibilityNodeProvider getAccessibilityNodeProvider() {
        // Ditto
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN) {
            return super.getAccessibilityNodeProvider();
        }

        // Copy code from JellyBeanContentView because the class is not public
        AccessibilityNodeProvider provider = mContentViewCore.getAccessibilityNodeProvider();
        if (provider != null) {
            return provider;
        } else {
            return super.getAccessibilityNodeProvider();
        }
    }

    @Override
    public boolean performLongClick(){
        return mXWalkView.onPerformLongClick();
    }
}
