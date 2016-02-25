// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;
import android.util.Pair;

import org.chromium.base.test.util.Feature;
import org.chromium.net.test.util.TestWebServer;

/**
 * Test suite for setOriginAccessWhitelist().
 */
public class SetOriginAccessWhitelistTest extends XWalkViewTestBase {
    private static final int WAIT_XML_REQUEST = 1000;
    private TestWebServer mWebServer;

    @Override
    public void setUp() throws Exception {
        super.setUp();
        mWebServer = TestWebServer.start(4444);
    }

    @Override
    public void tearDown() throws Exception {
        mWebServer.shutdown();
        super.tearDown();
    }

    @SmallTest
    @Feature({"setOriginAccessWhitelist"})
    public void testSetOriginAccessWhitelist() throws Throwable {
        final String loadUrl = "file:///android_asset/www/cross_origin.html";
        final String patterns = "http://localhost:4444/*,http://localhost:3333/*";
        // The server will be accessed by XMLHttpRequest from js.
        final String path = "/cross_origin_xhr_test.html";
        // The original title of the cross_origin.html.
        final String originalTitle = "Original Title";
        final String responseStr = "Cross-Origin XHR";
        final String url = mWebServer.setResponse(path, responseStr, null);
        assertEquals("http://localhost:4444/cross_origin_xhr_test.html", url);

        // Test without setOriginAccessWhitelist.
        loadUrlAsync(loadUrl);
        Thread.sleep(WAIT_XML_REQUEST);
        // XHR in page should be failed, and the title should be "Original Title".
        assertEquals(originalTitle, getTitleOnUiThread());

        // Test setOriginAccessWhitelist.
        setOriginAccessWhitelist(loadUrl, patterns.split(","));
        loadUrlAsync(loadUrl);
        Thread.sleep(WAIT_XML_REQUEST);
        // XHR in page should be success, and the title should be "Cross-Origin XHR".
        assertEquals(responseStr, getTitleOnUiThread());

        // Reset the Whitelist
        setOriginAccessWhitelist(loadUrl, null);
        loadUrlAsync(loadUrl);
        Thread.sleep(WAIT_XML_REQUEST);
        assertEquals(originalTitle, getTitleOnUiThread());
    }
}
