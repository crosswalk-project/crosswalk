// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.graphics.Bitmap;
import android.content.Context;
import android.test.suitebuilder.annotation.SmallTest;
import android.test.suitebuilder.annotation.MediumTest;

import java.util.concurrent.Callable;

import org.apache.http.Header;
import org.apache.http.HttpRequest;
import org.chromium.base.test.util.Feature;
import org.chromium.net.test.util.TestWebServer;
import org.xwalk.core.XWalkClient;
import org.xwalk.core.XWalkSettings;
import org.xwalk.core.XWalkView;

/**
 * Test suite for setUserAgentString() and getUserAgentString().
 */
public class UserAgentTest extends XWalkViewTestBase {
    @Override
    public void setUp() throws Exception {
        super.setUp();

        setXWalkClient(new XWalkViewTestBase.TestXWalkClient());
    }

    protected XWalkSettings getXWalkSettingsOnUiThread(
            ) throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<XWalkSettings>() {
            @Override
            public XWalkSettings call() throws Exception {
                return getXWalkView().getSettings();
            }
        });
    }

    @SmallTest
    @Feature({"UserAgent"})
    public void testUserAgent() throws Throwable {
        XWalkSettings settings = getXWalkSettingsOnUiThread();
        final String defaultUserAgentString = settings.getUserAgentString();

        // Check that an attempt to set the default UA string to null or "" has no effect.
        settings.setUserAgentString(null);
        assertEquals(defaultUserAgentString, settings.getUserAgentString());
        settings.setUserAgentString("");
        assertEquals(defaultUserAgentString, settings.getUserAgentString());

        // Check that we can also set the default value.
        settings.setUserAgentString(defaultUserAgentString);
        assertEquals(defaultUserAgentString, settings.getUserAgentString());

        // Set a custom UA string, verify that it can be reset back to default.
        final String customUserAgentString = "XWalkUserAgentTest";
        settings.setUserAgentString(customUserAgentString);
        assertEquals(customUserAgentString, settings.getUserAgentString());
        settings.setUserAgentString(null);
        assertEquals(defaultUserAgentString, settings.getUserAgentString());
    }

    @MediumTest
    @Feature({"UserAgent"})
    public void testUserAgentWithTestServer() throws Throwable {
        XWalkSettings settings = getXWalkSettingsOnUiThread();
        final String customUserAgentString =
                "testUserAgentWithTestServerUserAgent";

        TestWebServer webServer = null;
        String fileName = null;
        try {
            webServer = new TestWebServer(false);
            final String httpPath = "/testUserAgentWithTestServer.html";
            final String url = webServer.setResponse(httpPath, "foo", null);

            settings.setUserAgentString(customUserAgentString);
            loadUrlSync(url);

            assertEquals(1, webServer.getRequestCount(httpPath));
            HttpRequest request = webServer.getLastRequest(httpPath);
            Header[] matchingHeaders  = request.getHeaders("User-Agent");
            assertEquals(1, matchingHeaders.length);

            Header header = matchingHeaders[0];
            assertEquals(customUserAgentString, header.getValue());
        } finally {
            if (webServer != null) webServer.shutdown();
        }
    }
}
