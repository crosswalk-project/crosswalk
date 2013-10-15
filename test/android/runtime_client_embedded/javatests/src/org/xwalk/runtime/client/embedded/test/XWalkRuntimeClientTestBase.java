// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.client.embedded.test;

import android.app.Activity;
import android.content.Context;
import android.test.ActivityInstrumentationTestCase2;

import java.util.concurrent.TimeUnit;

import org.chromium.content.browser.test.util.CallbackHelper;
import org.xwalk.app.runtime.XWalkRuntimeClient;

public class XWalkRuntimeClientTestBase
        extends ActivityInstrumentationTestCase2<XWalkRuntimeClientTestRunnerActivity> {
    protected final static int WAIT_TIMEOUT_SECONDS = 15;
    private XWalkRuntimeClient mRuntimeView;
    final static TestXWalkRuntimeClientContentsClient mTestContentsClient =
        new TestXWalkRuntimeClientContentsClient();

    class PageStatusCallback {
        public void onPageStarted(String url) {
            mTestContentsClient.onPageStarted(url);
        }

        public void onPageFinished(String url) {
            mTestContentsClient.didFinishLoad(url);
        }

        public void onReceivedError(int errorCode, String description, String failingUrl) {
            mTestContentsClient.onReceivedError(errorCode, description, failingUrl);
        }
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        final Activity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                if (mRuntimeView == null || mRuntimeView.get() == null) {
                    mRuntimeView = new XWalkRuntimeClient(activity, null, null);
                }
                mRuntimeView.onCreate();
                PageStatusCallback callback = new PageStatusCallback();
                mRuntimeView.setCallbackForTest((Object)callback);
                getActivity().addView(mRuntimeView.getViewForTest());
            }
        });
    }

    public XWalkRuntimeClientTestBase() {
        super(XWalkRuntimeClientTestRunnerActivity.class);
    }

    protected void loadUrlSync(String url) throws Exception {
        CallbackHelper pageFinishedHelper = mTestContentsClient.getOnPageFinishedHelper();
        int currentCallCount = pageFinishedHelper.getCallCount();
        loadUrlAsync(url);

        pageFinishedHelper.waitForCallback(currentCallCount, 1, WAIT_TIMEOUT_SECONDS,
                TimeUnit.SECONDS);
    }

    protected void loadUrlAsync(final String url) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mRuntimeView.loadAppFromUrl(url);
            }
        });
    }

    protected XWalkRuntimeClient getRuntimeView() {
        return mRuntimeView;
    }
}
