// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.content.Context;
import android.test.ActivityInstrumentationTestCase2;

import java.util.concurrent.Callable;
import java.util.concurrent.FutureTask;
import java.util.concurrent.TimeUnit;
import java.util.Timer;

import org.chromium.content.browser.test.util.CallbackHelper;
import org.xwalk.core.XWalkContent;
import org.xwalk.core.XWalkView;

public class XWalkViewTestBase
       extends ActivityInstrumentationTestCase2<XWalkViewTestRunnerActivity> {
    protected final static int WAIT_TIMEOUT_SECONDS = 15;
    private XWalkView mXWalkView;
    final TestXWalkViewContentsClient mTestContentsClient = new TestXWalkViewContentsClient();

    public XWalkViewTestBase() {
        super(XWalkViewTestRunnerActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        final Context context = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView = new XWalkView(getActivity(), getActivity());
                getActivity().addView(mXWalkView);
                mXWalkView.getXWalkViewContentForTest().installWebContentsObserverForTest(mTestContentsClient);
            }
        });
    }

    protected void loadUrlSync(final String url) throws Exception {
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
                mXWalkView.loadUrl(url);
            }
        });
    }

    protected String getTitleOnUiThread() throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<String>() {
            @Override
            public String call() throws Exception {
                XWalkContent xWalkContent = mXWalkView.getXWalkViewContentForTest();
                String title = xWalkContent.getContentViewCoreForTest().getTitle();
                return title;
            }
        });
    }

    public <R> R runTestOnUiThreadAndGetResult(Callable<R> callable)
            throws Exception {
        FutureTask<R> task = new FutureTask<R>(callable);
        getInstrumentation().waitForIdleSync();
        getInstrumentation().runOnMainSync(task);
        return task.get();
    }

    protected XWalkView getXWalkView() {
        return mXWalkView;
    }
}
