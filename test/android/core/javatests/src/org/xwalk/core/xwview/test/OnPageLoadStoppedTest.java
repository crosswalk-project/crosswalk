// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.MediumTest;

import java.util.concurrent.TimeUnit;

import org.chromium.base.test.util.Feature;
import org.chromium.content.browser.test.util.TestCallbackHelperContainer;
import org.chromium.net.test.util.TestWebServer;

import org.xwalk.core.XWalkUIClient.LoadStatus;

import static org.chromium.base.test.util.ScalableTimeout.scaleTimeout;

/**
 * Tests for the XWalkUIClient.onPageLoadStopped() method.
 */
public class OnPageLoadStoppedTest extends XWalkViewTestBase {
    private static final long WAIT_TIMEOUT_MS = scaleTimeout(2000);
    TestWebServer mWebServer;
    TestCallbackHelperContainer.OnPageFinishedHelper mOnPageFinishedHelper;
    TestCallbackHelperContainer.OnReceivedErrorHelper mOnReceivedErrorHelper;
    TestCallbackHelperContainer.OnPageStartedHelper mOnPageStartedHelper;

    public void setUp() throws Exception {
        super.setUp();
        mWebServer = TestWebServer.start();
        mOnPageFinishedHelper = mTestHelperBridge.getOnPageFinishedHelper();
        mOnReceivedErrorHelper = mTestHelperBridge.getOnReceivedErrorHelper();
        mOnPageStartedHelper = mTestHelperBridge.getOnPageStartedHelper();
    }

    @Override
    protected void tearDown() throws Exception {
        mWebServer.shutdown();
        super.tearDown();
    }

    @MediumTest
    @Feature({"OnPageLoadStopped"})
    public void testOnPageLoadStoppedWithLocalUrl() throws Throwable {
        String url = "file:///android_asset/www/index.html";
        int currentCallCount = mOnPageFinishedHelper.getCallCount();
        loadUrlAsync(url);

        mOnPageFinishedHelper.waitForCallback(currentCallCount);
        assertEquals(url, mOnPageFinishedHelper.getUrl());
        assertEquals(LoadStatus.FINISHED, mTestHelperBridge.getLoadStatus());
    }

    @MediumTest
    @Feature({"OnPageLoadStopped"})
    public void testOnPageLoadStoppedWithServer() throws Throwable {
        final String testHtml = "<html><head>Header</head><body>Body</body></html>";
        final String testPath = "/test.html";

        final String testUrl = mWebServer.setResponse(testPath, testHtml, null);
        int currentCallCount = mOnPageFinishedHelper.getCallCount();
        loadUrlAsync(testUrl);

        mOnPageFinishedHelper.waitForCallback(currentCallCount);
        assertEquals(testUrl, mOnPageFinishedHelper.getUrl());
        assertEquals(LoadStatus.FINISHED, mTestHelperBridge.getLoadStatus());
    }

    @MediumTest
    @Feature({"OnPageLoadStopped"})
    public void testOnPageLoadStoppedWithData() throws Throwable {
        final String name = "index.html";
        String fileContent = getFileContent(name);
        int currentCallCount = mOnPageFinishedHelper.getCallCount();
        loadDataAsync(null, fileContent, "text/html", false);

        mOnPageFinishedHelper.waitForCallback(currentCallCount);
        assertEquals("about:blank", mOnPageFinishedHelper.getUrl());
        assertEquals(LoadStatus.FINISHED, mTestHelperBridge.getLoadStatus());
    }

    @MediumTest
    @Feature({"OnPageLoadStopped"})
    public void testOnPageLoadStoppedWithInvalidUrl() throws Throwable {
        String url = "http://localhost/non_existent";
        int currentCallCount = mOnPageFinishedHelper.getCallCount();
        int onReceivedErrorCallCount = mOnReceivedErrorHelper.getCallCount();
        assertEquals(0, mOnReceivedErrorHelper.getCallCount());
        loadUrlAsync(url);

        mOnReceivedErrorHelper.waitForCallback(onReceivedErrorCallCount,
                                               1, WAIT_TIMEOUT_MS,
                                               TimeUnit.MILLISECONDS);
        mOnPageFinishedHelper.waitForCallback(currentCallCount,
                                              1, WAIT_TIMEOUT_MS,
                                              TimeUnit.MILLISECONDS);
        assertEquals(1, mOnReceivedErrorHelper.getCallCount());
        assertEquals(url, mOnPageFinishedHelper.getUrl());
        assertEquals(LoadStatus.FAILED, mTestHelperBridge.getLoadStatus());
    }

    @MediumTest
    @Feature({"OnPageLoadStopped"})
    public void testOnPageLoadStoppedWithStopLoading() throws Throwable {
        final String testHtml = "<html><head>Header</head><body>Body</body></html>";
        final String testPath = "/test.html";

        final String testUrl = mWebServer.setResponse(testPath, testHtml, null);
        int currentCallCount = mOnPageFinishedHelper.getCallCount();
        int startedCount = mOnPageStartedHelper.getCallCount();
        loadUrlAsync(testUrl);
        mOnPageStartedHelper.waitForCallback(startedCount);
        stopLoading();
        mOnPageFinishedHelper.waitForCallback(currentCallCount);
        assertEquals(testUrl, mOnPageFinishedHelper.getUrl());
        assertEquals(LoadStatus.CANCELLED, mTestHelperBridge.getLoadStatus());
    }
}
