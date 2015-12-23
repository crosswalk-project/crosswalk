// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;
import android.util.Pair;

import java.util.ArrayList;
import java.util.List;

import org.apache.http.HttpRequest;
import org.chromium.base.test.util.Feature;
import org.chromium.net.test.util.TestWebServer;
import org.xwalk.core.XWalkCookieManager;

/**
 * Test suite for DownloadListener().
 */
public class DownloadListenerTest extends XWalkViewTestBase {
    private TestHelperBridge.OnDownloadStartHelper mDownloadStartHelper =
            mTestHelperBridge.getOnDownloadStartHelper();

    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @SmallTest
    @Feature({"onDownloadStart"})
    public void testOnDownloadStart() throws Throwable {
        final String data = "download data";
        final String contentDisposition = "attachment;filename=\"download.txt\"";
        final String mimeType = "text/plain";
        final String userAgent = "Chrome/44.0.2403.81 Crosswalk/15.44.376.0 Mobile Safari/537.36";
        final String cookieValue = "cookie data";

        List<Pair<String, String>> downloadHeaders = new ArrayList<Pair<String, String>>();
        downloadHeaders.add(Pair.create("Content-Disposition", contentDisposition));
        downloadHeaders.add(Pair.create("Content-Type", mimeType));
        downloadHeaders.add(Pair.create("Content-Length", Integer.toString(data.length())));

        setUserAgent(userAgent);
        setDownloadListener();
        XWalkCookieManager cookieManager = new XWalkCookieManager();
        TestWebServer webServer = TestWebServer.start();
        try {
            final String requestPath = "/download.txt";
            final String pageUrl = webServer.setResponse(requestPath, data, downloadHeaders);
            final int callCount = mDownloadStartHelper.getCallCount();
            cookieManager.setCookie(pageUrl, cookieValue);
            loadUrlAsync(pageUrl);
            mDownloadStartHelper.waitForCallback(callCount);

            assertEquals(pageUrl, mDownloadStartHelper.getUrl());
            assertEquals(contentDisposition, mDownloadStartHelper.getContentDisposition());
            assertEquals(mimeType, mDownloadStartHelper.getMimeType());
            assertEquals(data.length(), mDownloadStartHelper.getContentLength());
            assertEquals(userAgent, mDownloadStartHelper.getUserAgent());

            final HttpRequest lastRequest = webServer.getLastRequest(requestPath);
            assertEquals(cookieValue, lastRequest.getFirstHeader("Cookie").getValue());
        } finally {
            webServer.shutdown();
        }

    }
}
