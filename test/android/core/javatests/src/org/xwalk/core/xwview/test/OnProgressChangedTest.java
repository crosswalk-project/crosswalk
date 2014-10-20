// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;
import org.chromium.net.test.util.TestWebServer;

/**
 * Test suite for onProgressChanged().
 */
public class OnProgressChangedTest extends XWalkViewTestBase {
    private TestHelperBridge.OnProgressChangedHelper mOnProgressChangedHelper;
    private TestWebServer mWebServer;

    @Override
    public void setUp() throws Exception {
        super.setUp();

        mOnProgressChangedHelper = mTestHelperBridge.getOnProgressChangedHelper();
        mWebServer = TestWebServer.start();
    }

    @Override
    public void tearDown() throws Exception {
        mWebServer.shutdown();
        super.tearDown();
    }

    @SmallTest
    @Feature({"OnProgressChanged"})
    public void testOnProgressChanged() throws Throwable {
        final int callCount = mOnProgressChangedHelper.getCallCount();
        final String testHtml = "<html><head>Header</head><body>Body</body></html>";
        final String testPath = "/test.html";
        final String testUrl = mWebServer.setResponse(testPath, testHtml, null);

        loadDataAsync(null, "<html><iframe src=\"" + testUrl + "\" /></html>",
                      "text/html",
                      false);

        mOnProgressChangedHelper.waitForCallback(callCount);
        assertTrue(mOnProgressChangedHelper.getProgress() > 0);
    }
}
