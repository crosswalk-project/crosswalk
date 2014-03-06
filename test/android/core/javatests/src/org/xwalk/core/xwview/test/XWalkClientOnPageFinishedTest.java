// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.MediumTest;

import static org.chromium.base.test.util.ScalableTimeout.scaleTimeout;

import org.chromium.base.test.util.Feature;
import org.chromium.content.browser.test.util.TestCallbackHelperContainer;
import org.chromium.net.test.util.TestWebServer;

import org.xwalk.core.XWalkClient;
import org.xwalk.core.XWalkContent;
import org.xwalk.core.XWalkView;

import java.util.concurrent.TimeUnit;

/**
 * Tests for the XWalkClient.onPageFinished() method.
 */
public class XWalkClientOnPageFinishedTest extends XWalkViewTestBase {
    private static final long WAIT_TIMEOUT_MS = scaleTimeout(2000);

    @Override
    public void setUp() throws Exception {
        super.setUp();

        class TestXWalkClient extends XWalkClient {
            @Override
            public void onPageFinished(XWalkView view, String url) {
                mTestContentsClient.didFinishLoad(url);
            }
        }

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getXWalkView().setXWalkClient(new TestXWalkClient());
            }
        });
    }

    @MediumTest
    @Feature({"XWalkClientOnPageFinishedTest"})
    public void testOnPageFinishedPassesCorrectUrl() throws Throwable {
        TestCallbackHelperContainer.OnPageFinishedHelper onPageFinishedHelper =
                mTestContentsClient.getOnPageFinishedHelper();

        String html = "<html><body>Simple page.</body></html>";
        int currentCallCount = onPageFinishedHelper.getCallCount();
        loadDataAsync(html, "text/html", false);

        onPageFinishedHelper.waitForCallback(currentCallCount);
        assertEquals("data:text/html," + html, onPageFinishedHelper.getUrl());
    }

    @MediumTest
    @Feature({"XWalkClientOnPageFinishedTest"})
    public void testOnPageFinishedCalledAfterError() throws Throwable {
        TestCallbackHelperContainer.OnReceivedErrorHelper onReceivedErrorHelper =
                mTestContentsClient.getOnReceivedErrorHelper();
        TestCallbackHelperContainer.OnPageFinishedHelper onPageFinishedHelper =
                mTestContentsClient.getOnPageFinishedHelper();

        assertEquals(0, onReceivedErrorHelper.getCallCount());

        String url = "http://localhost:7/non_existent";
        int onReceivedErrorCallCount = onReceivedErrorHelper.getCallCount();
        int onPageFinishedCallCount = onPageFinishedHelper.getCallCount();
        loadUrlAsync(url);

        onReceivedErrorHelper.waitForCallback(onReceivedErrorCallCount,
                                              1 /* numberOfCallsToWaitFor */,
                                              WAIT_TIMEOUT_MS,
                                              TimeUnit.MILLISECONDS);
        onPageFinishedHelper.waitForCallback(onPageFinishedCallCount,
                                             1 /* numberOfCallsToWaitFor */,
                                             WAIT_TIMEOUT_MS,
                                             TimeUnit.MILLISECONDS);
        assertEquals(1, onReceivedErrorHelper.getCallCount());
    }

    @MediumTest
    @Feature({"XWalkClientOnPageFinishedTest"})
    public void testOnPageFinishedNotCalledForValidSubresources() throws Throwable {
        TestCallbackHelperContainer.OnPageFinishedHelper onPageFinishedHelper =
                mTestContentsClient.getOnPageFinishedHelper();

        TestWebServer webServer = null;
        try {
            webServer = new TestWebServer(false);

            final String testHtml = "<html><head>Header</head><body>Body</body></html>";
            final String testPath = "/test.html";
            final String syncPath = "/sync.html";

            final String testUrl = webServer.setResponse(testPath, testHtml, null);
            final String syncUrl = webServer.setResponse(syncPath, testHtml, null);

            assertEquals(0, onPageFinishedHelper.getCallCount());
            final int pageWithSubresourcesCallCount = onPageFinishedHelper.getCallCount();
            loadDataAsync("<html><iframe src=\"" + testUrl + "\" /></html>",
                          "text/html",
                          false);

            onPageFinishedHelper.waitForCallback(pageWithSubresourcesCallCount);

            // Rather than wait a fixed time to see that an onPageFinished callback isn't issued
            // we load another valid page. Since callbacks arrive sequentially if the next callback
            // we get is for the synchronizationUrl we know that the previous load did not schedule
            // a callback for the iframe.
            final int synchronizationPageCallCount = onPageFinishedHelper.getCallCount();
            loadUrlAsync(syncUrl);

            onPageFinishedHelper.waitForCallback(synchronizationPageCallCount);
            assertEquals(syncUrl, onPageFinishedHelper.getUrl());
            assertEquals(2, onPageFinishedHelper.getCallCount());
        } finally {
            if (webServer != null) webServer.shutdown();
        }
    }

    @MediumTest
    @Feature({"XWalkClientOnPageFinishedTest"})
    public void testOnPageFinishedNotCalledForJavaScriptUrl() throws Throwable {
        TestCallbackHelperContainer.OnPageFinishedHelper onPageFinishedHelper =
                mTestContentsClient.getOnPageFinishedHelper();

        String html = "<html><body>Simple page.</body></html>";
        int currentCallCount = onPageFinishedHelper.getCallCount();
        assertEquals(0, currentCallCount);

        loadDataAsync(html, "text/html", false);
        loadJavaScriptUrl("javascript: try { console.log('foo'); } catch(e) {};");

        onPageFinishedHelper.waitForCallback(currentCallCount);
        assertEquals("data:text/html," + html, onPageFinishedHelper.getUrl());
        // onPageFinished won't be called for javascript: url.
        assertEquals(1, onPageFinishedHelper.getCallCount());
    }
}
