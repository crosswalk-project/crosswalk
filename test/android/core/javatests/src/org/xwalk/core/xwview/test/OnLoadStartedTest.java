// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;
import org.chromium.net.test.util.TestWebServer;

/**
 * Test suite for onLoadStarted().
 */
public class OnLoadStartedTest extends XWalkViewTestBase {
    private TestHelperBridge.OnLoadStartedHelper mOnLoadStartedHelper;
    private TestWebServer mWebServer;

    @Override
    public void setUp() throws Exception {
        super.setUp();

        mOnLoadStartedHelper = mTestHelperBridge.getOnLoadStartedHelper();
        mWebServer = new TestWebServer(false);
    }

    @Override
    public void tearDown() throws Exception {
        if (mWebServer != null) {
            mWebServer.shutdown();
        }
        super.tearDown();
    }

    @SmallTest
    @Feature({"OnLoadStarted"})
    public void testOnLoadStarted() throws Throwable {
        final int callCount = mOnLoadStartedHelper.getCallCount();
        final String testHtml = "<html><head>Header</head><body>Body</body></html>";
        final String testPath = "/test.html";
        final String testUrl = mWebServer.setResponse(testPath, testHtml, null);

        loadDataAsync(null, "<html><iframe src=\"" + testUrl + "\" /></html>",
                      "text/html",
                      false);

        mOnLoadStartedHelper.waitForCallback(callCount);
        assertEquals(testUrl, mOnLoadStartedHelper.getUrl());
    }
}
