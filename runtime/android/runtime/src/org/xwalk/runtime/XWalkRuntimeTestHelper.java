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
import org.xwalk.core.XWalkView;
import org.xwalk.core.internal.XWalkClient;
import org.xwalk.core.internal.XWalkViewInternal;
import org.xwalk.core.internal.XWalkWebChromeClient;

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

    class TestXWalkClient extends XWalkClient {
        TestXWalkClient(Context context, XWalkView view) {
            super(view);
        }

        @Override
        public void onPageStarted(XWalkViewInternal view, String url) {
            super.onPageStarted(view, url);
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
        public void onPageFinished(XWalkViewInternal view, String url) {
            super.onPageFinished(view, url);
            if (mCallbackForTest != null) {
                try {
                    Class<?> objectClass = mCallbackForTest.getClass();
                    Method onPageStarted = objectClass.getMethod("onPageFinished", String.class);
                    onPageStarted.invoke(mCallbackForTest, url);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }

    class TestXWalkWebChromeClient extends XWalkWebChromeClient {
        TestXWalkWebChromeClient(Context context, XWalkView view) {
            super(view);
        }

        @Override
        public void onReceivedTitle(XWalkViewInternal view, String title) {
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
    private TestXWalkClient mClient;
    private TestXWalkWebChromeClient mWebChromeClient;
    private TestXWalkResourceClient mResourceClient;

    XWalkRuntimeTestHelper(Context context, XWalkView view) {
        mClient = new TestXWalkClient(context, view);
        mWebChromeClient = new TestXWalkWebChromeClient(context, view);
        mResourceClient = new TestXWalkResourceClient(context, view);
    }

    void setCallbackForTest(Object callback) {
        mCallbackForTest = callback;
    }

    XWalkClient getClient() {
        return mClient;
    }

    XWalkWebChromeClient getWebChromeClient() {
        return mWebChromeClient;
    }

    XWalkResourceClient getResourceClient() {
        return mResourceClient;
    }
}
