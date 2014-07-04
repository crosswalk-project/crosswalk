// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime;

import android.content.Context;
import android.graphics.Bitmap;
import android.net.http.SslError;
import android.webkit.ValueCallback;

import java.lang.reflect.Method;

import org.xwalk.core.XWalkResourceClient;
import org.xwalk.core.XWalkUIClient;
import org.xwalk.core.XWalkView;

class XWalkRuntimeTestHelper {

    class TestXWalkResourceClient extends XWalkResourceClient {
        TestXWalkResourceClient(Context context, XWalkView view) {
            super(view);
        }

        @Override
        public void onReceivedLoadError(XWalkView view, int errorCode,
                String description, String failingUrl) {
            super.onReceivedLoadError(view, errorCode, description, failingUrl);
            if (mCallbackForTest != null) {
                try {
                    Class<?> objectClass = mCallbackForTest.getClass();
                    Method onReceivedLoadError = objectClass.getMethod(
                            "onReceivedLoadError", int.class, String.class, String.class);
                    onReceivedLoadError.invoke(mCallbackForTest, errorCode, description,
                            failingUrl);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }

    class TestXWalkUIClient extends XWalkUIClient {
        TestXWalkUIClient(Context context, XWalkView view) {
            super(view);
        }

        @Override
        public void onPageLoadStarted(XWalkView view, String url) {
            super.onPageLoadStarted(view, url);
            if (mCallbackForTest != null) {
                try {
                    Class<?> objectClass = mCallbackForTest.getClass();
                    Method onPageStarted = objectClass.getMethod("onPageStarted", String.class);
                    onPageStarted.invoke(mCallbackForTest, url);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void onPageLoadStopped(XWalkView view, String url, LoadStatus status) {
            super.onPageLoadStopped(view, url, status);
            if (mCallbackForTest != null) {
                try {
                    Class<?> objectClass = mCallbackForTest.getClass();
                    Method onPageFinished = objectClass.getMethod("onPageFinished", String.class);
                    onPageFinished.invoke(mCallbackForTest, url);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void onReceivedTitle(XWalkView view, String title) {
            super.onReceivedTitle(view, title);
            if (mCallbackForTest != null) {
                try {
                    Class<?> objectClass = mCallbackForTest.getClass();
                    Method onReceivedTitle = objectClass.getMethod("onReceivedTitle", String.class);
                    onReceivedTitle.invoke(mCallbackForTest, title);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private Object mCallbackForTest;
    private TestXWalkUIClient mUIClient;
    private TestXWalkResourceClient mResourceClient;

    XWalkRuntimeTestHelper(Context context, XWalkView view) {
        mUIClient = new TestXWalkUIClient(context, view);
        mResourceClient = new TestXWalkResourceClient(context, view);
    }

    void setCallbackForTest(Object callback) {
        mCallbackForTest = callback;
    }

    XWalkUIClient getUIClient() {
        return mUIClient;
    }

    XWalkResourceClient getResourceClient() {
        return mResourceClient;
    }
}
