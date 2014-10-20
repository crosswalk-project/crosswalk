// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.MediumTest;
import android.util.Pair;

import java.util.ArrayList;
import java.util.List;

import org.chromium.base.test.util.Feature;
import org.chromium.content.browser.test.util.TestCallbackHelperContainer;
import org.chromium.net.test.util.TestWebServer;

import org.xwalk.core.xwview.test.util.CommonResources;

/**
 * Tests for the XWalkUIClient.OnReceivedTitleTest() method.
 */
public class OnReceivedTitleTest extends XWalkViewTestBase {
    private OnTitleUpdatedHelper mOnTitleUpdatedHelper;
    private TestWebServer mWebServer;

    @Override
    public void setUp() throws Exception {
        super.setUp();

        mOnTitleUpdatedHelper = mTestHelperBridge.getOnTitleUpdatedHelper();
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

    @MediumTest
    @Feature({"onReceivedTitle"})
    public void testOnReceivedTitleWithUrl() throws Throwable {
        String path = "/test.html";
        String pageContent = CommonResources.makeHtmlPageFrom("<title>Test</title>",
                "<div> The title is: Test </div>");
        String url = addPageToTestServer(mWebServer, path, pageContent);
        int onReceivedTitleCallCount = mOnTitleUpdatedHelper.getCallCount();

        loadUrlAsync(url);
        mOnTitleUpdatedHelper.waitForCallback(onReceivedTitleCallCount);
        assertNotNull(mOnTitleUpdatedHelper.getTitle());
    }

    @MediumTest
    @Feature({"onReceivedTitle"})
    public void testOnReceivedTitleWithData() throws Throwable {
        final String name = "index.html";
        final String fileContent = getFileContent(name);
        int onReceivedTitleCallCount = mOnTitleUpdatedHelper.getCallCount();

        loadDataSync(name, fileContent, "text/html", false);
        mOnTitleUpdatedHelper.waitForCallback(onReceivedTitleCallCount);
        assertNotNull(mOnTitleUpdatedHelper.getTitle());
    }
}
