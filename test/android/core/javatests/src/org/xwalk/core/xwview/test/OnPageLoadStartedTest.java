// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.MediumTest;

import org.chromium.base.test.util.Feature;
import org.chromium.content.browser.test.util.TestCallbackHelperContainer;
import org.chromium.net.test.util.TestWebServer;

/**
 * Tests for the XWalkUIClient.onPageLoadStarted() method.
 */
public class OnPageLoadStartedTest extends XWalkViewTestBase {
    TestCallbackHelperContainer.OnPageStartedHelper mOnPageStartedHelper;

    public void setUp() throws Exception {
        super.setUp();
        mOnPageStartedHelper = mTestHelperBridge.getOnPageStartedHelper();
    }

    @MediumTest
    @Feature({"OnPageLoadStarted"})
    public void testOnPageLoadStartedWithLocalUrl() throws Throwable {
        String url = "file:///android_asset/www/index.html";
        int currentCallCount = mOnPageStartedHelper.getCallCount();
        loadUrlAsync(url);

        mOnPageStartedHelper.waitForCallback(currentCallCount);
        assertEquals(url, mOnPageStartedHelper.getUrl());
    }

    @MediumTest
    @Feature({"OnPageLoadStarted"})
    public void testOnPageLoadStartedWithServer() throws Throwable {
        TestWebServer webServer = TestWebServer.start();
        try {
            final String testHtml = "<html><head>Header</head><body>Body</body></html>";
            final String testPath = "/test.html";

            final String testUrl = webServer.setResponse(testPath, testHtml, null);
            int currentCallCount = mOnPageStartedHelper.getCallCount();
            loadUrlAsync(testUrl);

            mOnPageStartedHelper.waitForCallback(currentCallCount);
            assertEquals(testUrl, mOnPageStartedHelper.getUrl());
        } finally {
            webServer.shutdown();
        }
    }

    @MediumTest
    @Feature({"OnPageLoadStarted"})
    public void testOnPageLoadStartedWithInvalidUrl() throws Throwable {
        String url = "http://this.url.is.invalid/";
        int currentCallCount = mOnPageStartedHelper.getCallCount();
        loadUrlAsync(url);

        mOnPageStartedHelper.waitForCallback(currentCallCount);
        assertEquals(url, mOnPageStartedHelper.getUrl());
    }
}
