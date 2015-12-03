// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;
import android.util.Pair;

import org.chromium.base.test.util.Feature;
import org.chromium.base.test.util.MinAndroidSdkLevel;
import org.chromium.net.test.util.TestWebServer;

import java.util.ArrayList;
import java.util.List;

import org.xwalk.core.XWalkWebResourceRequest;
import org.xwalk.core.XWalkWebResourceResponse;
import org.xwalk.core.xwview.test.util.CommonResources;

/**
 * Tests for the XWalkResourceClient.onReceivedResponseHeaders() method.
 */
public class OnReceivedResponseHeadersTest extends XWalkViewTestBase {

    private TestHelperBridge.OnReceivedResponseHeadersHelper mOnReceivedResponseHeadersHelper;
    private TestWebServer mWebServer;

    @Override
    public void setUp() throws Exception {
        super.setUp();
        mOnReceivedResponseHeadersHelper = mTestHelperBridge.getOnReceivedResponseHeadersHelper();
        mWebServer = TestWebServer.start();
    }

    @Override
    protected void tearDown() throws Exception {
        if (mWebServer != null) mWebServer.shutdown();
        super.tearDown();
    }

    @SmallTest
    @Feature({"OnReceivedResponseHeaders"})
    public void testForMainResource() throws Throwable {
        List<Pair<String, String>> headers = new ArrayList<Pair<String, String>>();
        headers.add(Pair.create("Content-Type", "text/html; charset=utf-8"));
        headers.add(Pair.create("Coalesce", ""));
        headers.add(Pair.create("Coalesce", "a"));
        headers.add(Pair.create("Coalesce", ""));
        headers.add(Pair.create("Coalesce", "a"));
        final String url = mWebServer.setResponseWithNotFoundStatus("/404.html", headers);
        loadUrlSync(url);

        XWalkWebResourceRequest request = mOnReceivedResponseHeadersHelper.getRequest();
        assertNotNull(request);
        assertEquals("GET", request.getMethod());
        assertNotNull(request.getRequestHeaders());
        assertFalse(request.getRequestHeaders().isEmpty());
        assertTrue(request.isForMainFrame());
        assertFalse(request.hasGesture());
        XWalkWebResourceResponse response = mOnReceivedResponseHeadersHelper.getResponse();
        assertEquals(404, response.getStatusCode());
        assertEquals("Not Found", response.getReasonPhrase());
        assertEquals("text/html", response.getMimeType());
        assertNotNull(response.getResponseHeaders());
        assertTrue(response.getResponseHeaders().containsKey("Content-Type"));
        assertEquals("text/html; charset=utf-8", response.getResponseHeaders().get("Content-Type"));
        assertTrue(response.getResponseHeaders().containsKey("Coalesce"));
        assertEquals("a, a", response.getResponseHeaders().get("Coalesce"));
    }

    @SmallTest
    @Feature({"OnReceivedResponseHeaders"})
    public void testForSubresource() throws Throwable {
        List<Pair<String, String>> headers = new ArrayList<Pair<String, String>>();
        headers.add(Pair.create("Content-Type", "text/html; charset=utf-8"));
        final String imageUrl = mWebServer.setResponseWithNotFoundStatus("/404.png", headers);
        final String pageHtml = CommonResources.makeHtmlPageFrom(
                "", "<img src='" + imageUrl + "' class='img.big' />");
        final String pageUrl = mWebServer.setResponse("/page.html", pageHtml, null);
        loadUrlSync(pageUrl);

        XWalkWebResourceRequest request = mOnReceivedResponseHeadersHelper.getRequest();
        assertNotNull(request);
        assertEquals("GET", request.getMethod());
        assertNotNull(request.getRequestHeaders());
        assertFalse(request.getRequestHeaders().isEmpty());
        assertFalse(request.isForMainFrame());
        assertFalse(request.hasGesture());
        XWalkWebResourceResponse response = mOnReceivedResponseHeadersHelper.getResponse();
        assertEquals(404, response.getStatusCode());
        assertEquals("Not Found", response.getReasonPhrase());
        assertEquals("text/html", response.getMimeType());
        assertNotNull(response.getResponseHeaders());
        assertTrue(response.getResponseHeaders().containsKey("Content-Type"));
        assertEquals("text/html; charset=utf-8", response.getResponseHeaders().get("Content-Type"));
    }

    @SmallTest
    @Feature({"OnReceivedResponseHeaders"})
    public void testAfterRedirect() throws Throwable {
        List<Pair<String, String>> headers = new ArrayList<Pair<String, String>>();
        headers.add(Pair.create("Content-Type", "text/html; charset=utf-8"));
        final String secondUrl = mWebServer.setResponseWithNotFoundStatus("/404.html", headers);
        final String firstUrl = mWebServer.setRedirect("/302.html", secondUrl);
        loadUrlSync(firstUrl);

        XWalkWebResourceRequest request = mOnReceivedResponseHeadersHelper.getRequest();
        assertNotNull(request);
        assertEquals("GET", request.getMethod());
        assertNotNull(request.getRequestHeaders());
        assertFalse(request.getRequestHeaders().isEmpty());
        assertTrue(request.isForMainFrame());
        assertFalse(request.hasGesture());
        XWalkWebResourceResponse response = mOnReceivedResponseHeadersHelper.getResponse();
        assertEquals(404, response.getStatusCode());
        assertEquals("Not Found", response.getReasonPhrase());
        assertEquals("text/html", response.getMimeType());
        assertNotNull(response.getResponseHeaders());
        assertTrue(response.getResponseHeaders().containsKey("Content-Type"));
        assertEquals("text/html; charset=utf-8", response.getResponseHeaders().get("Content-Type"));
    }
}
