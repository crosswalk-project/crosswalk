// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime;

import android.content.Context;
import android.graphics.Bitmap;

import java.lang.reflect.Method;

import org.xwalk.core.client.XWalkDefaultClient;
import org.xwalk.core.XWalkView;

public class XWalkClientForTest extends XWalkDefaultClient {
    private Object mCallbackForTest;

    public XWalkClientForTest(Context context, XWalkView view) {
        super(context, view);
    }

    @Override
    public void onReceivedError(XWalkView view, int errorCode,
            String description, String failingUrl) {
        if (mCallbackForTest != null) {
            try {
                Class objectClass = mCallbackForTest.getClass();
                Method onReceivedError = objectClass.getMethod("onReceivedError", int.class, String.class, String.class);
                onReceivedError.invoke(mCallbackForTest, errorCode, description, failingUrl);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public void onPageStarted(XWalkView view, String url, Bitmap favicon) {
        if (mCallbackForTest != null) {
            try {
                Class objectClass = mCallbackForTest.getClass();
                Method onPageStarted = objectClass.getMethod("onPageStarted", String.class);
                onPageStarted.invoke(mCallbackForTest, url);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public void onPageFinished(XWalkView view, String url) {
        if (mCallbackForTest != null) {
            try {
                Class objectClass = mCallbackForTest.getClass();
                Method onPageStarted = objectClass.getMethod("onPageFinished", String.class);
                onPageStarted.invoke(mCallbackForTest, url);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    public void setCallbackForTest(Object callback) {
        mCallbackForTest = callback;
    }
}
