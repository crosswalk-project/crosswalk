// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime;

import android.content.Context;

import java.lang.reflect.Method;

import org.xwalk.core.client.XWalkDefaultWebChromeClient;
import org.xwalk.core.XWalkView;

public class XWalkWebChromeClientForTest extends XWalkDefaultWebChromeClient{
    private Object mCallbackForTest;

    public XWalkWebChromeClientForTest(Context context, XWalkView view) {
        super(context, view);
    }

    public void setCallbackForTest(Object callback) {
        mCallbackForTest = callback;
    }

    @Override
    public void onReceivedTitle(XWalkView view, String title) {
        if (mCallbackForTest != null) {
            try {
                Class objectClass = mCallbackForTest.getClass();
                Method onReceivedTitle = objectClass.getMethod("onReceivedTitle", String.class);
                onReceivedTitle.invoke(mCallbackForTest, title);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
}
