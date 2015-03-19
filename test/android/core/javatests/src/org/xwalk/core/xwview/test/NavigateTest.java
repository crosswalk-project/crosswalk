// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;
import android.util.Pair;

import java.util.ArrayList;
import java.util.List;

import org.chromium.base.test.util.Feature;
import org.chromium.net.test.util.TestWebServer;

import org.xwalk.core.xwview.test.util.CommonResources;

/**
 * Test suite for Navigate().
 */
public class NavigateTest extends XWalkViewTestBase {
    private TestWebServer mWebServer;

    @Override
    public void setUp() throws Exception {
        super.setUp();
        mWebServer = TestWebServer.start();
    }

    @Override
    protected void tearDown() throws Exception {
        mWebServer.shutdown();
        super.tearDown();
    }

    private String addPageToTestServer(TestWebServer webServer, String httpPath, String html) {
        List<Pair<String, String>> headers = new ArrayList<Pair<String, String>>();
        headers.add(Pair.create("Content-Type", "text/html"));
        headers.add(Pair.create("Cache-Control", "no-store"));
        return webServer.setResponse(httpPath, html, headers);
    }

    @SmallTest
    @Feature({"Navigate"})
    public void testNavigate() throws Throwable {
        String path = "/test.html";
        String pageContent = CommonResources.makeHtmlPageFrom("<title>Test</title>",
                "<div> The title is: Test </div>");
        final String firstUrl = addPageToTestServer(mWebServer, path, pageContent);
        final String firstTitle = "Test";
        final String secondUrl = "file:///android_asset/www/index.html";
        final String secondTitle ="Crosswalk Sample Application";

        loadUrlSync(firstUrl);
        loadUrlSync(secondUrl);
        assertEquals(secondUrl, getUrlOnUiThread());
        assertEquals(secondTitle, getTitleOnUiThread());
        goBackSync();
        assertEquals(firstUrl, getUrlOnUiThread());
        assertEquals(firstTitle, getTitleOnUiThread());
        goForwardSync();
        assertEquals(secondUrl, getUrlOnUiThread());
        assertEquals(secondTitle, getTitleOnUiThread());
    }
}
