// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime;

import android.content.Context;
import android.graphics.Bitmap;
import android.net.http.SslError;

import java.lang.reflect.Method;

import org.xwalk.core.client.XWalkDefaultClient;
import org.xwalk.core.HttpAuthHandler;
import org.xwalk.core.SslErrorHandler;
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
                Method onReceivedError = objectClass.getMethod(
                        "onReceivedError", int.class, String.class, String.class);
                onReceivedError.invoke(mCallbackForTest, errorCode, description, failingUrl);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public void onReceivedSslError(XWalkView view, SslErrorHandler handler,
            SslError error) {
        if (mCallbackForTest != null) {
            try {
                Class objectClass = mCallbackForTest.getClass();
                Method onReceivedSslError = objectClass.getMethod(
                        "onReceivedSslError", SslErrorHandler.class, SslError.class);
                onReceivedSslError.invoke(mCallbackForTest, handler, error);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public void onReceivedHttpAuthRequest(XWalkView view, HttpAuthHandler handler,
            String hosts, String realm) {
        if (mCallbackForTest != null) {
            try {
                Class objectClass = mCallbackForTest.getClass();
                Method onReceivedHttpAuthRequest = objectClass.getMethod(
                        "onReceivedHttpAuthRequest", HttpAuthHandler.class, String.class, String.class);
                onReceivedHttpAuthRequest.invoke(mCallbackForTest, handler, hosts, realm);
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
