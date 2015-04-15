// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;
import android.util.Pair;

import java.util.ArrayList;
import java.util.List;

import org.chromium.base.test.util.Feature;
import org.chromium.net.test.util.TestWebServer;

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

        List<Pair<String, String>> downloadHeaders = new ArrayList<Pair<String, String>>();
        downloadHeaders.add(Pair.create("Content-Disposition", contentDisposition));
        downloadHeaders.add(Pair.create("Content-Type", mimeType));
        downloadHeaders.add(Pair.create("Content-Length", Integer.toString(data.length())));

        setDownloadListener();
        TestWebServer webServer = TestWebServer.start();
        try {
            final String pageUrl = webServer.setResponse("/download.txt", data, downloadHeaders);
            final int callCount = mDownloadStartHelper.getCallCount();
            loadUrlAsync(pageUrl);
            mDownloadStartHelper.waitForCallback(callCount);

            assertEquals(pageUrl, mDownloadStartHelper.getUrl());
            assertEquals(contentDisposition, mDownloadStartHelper.getContentDisposition());
            assertEquals(mimeType, mDownloadStartHelper.getMimeType());
            assertEquals(data.length(), mDownloadStartHelper.getContentLength());
        } finally {
            webServer.shutdown();
        }

    }
}
