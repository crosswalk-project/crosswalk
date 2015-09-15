// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.MediumTest;
import android.test.suitebuilder.annotation.SmallTest;

import org.apache.http.Header;
import org.apache.http.HttpRequest;
import org.chromium.base.test.util.Feature;
import org.chromium.net.test.util.TestWebServer;

/**
 * Test suite for setUserAgentString() and getUserAgentString().
 */
public class UserAgentTest extends XWalkViewTestBase {
    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @SmallTest
    @Feature({"UserAgent"})
    public void testUserAgent() throws Throwable {
        final String USER_AGENT = "Chrome/44.0.2403.81 Crosswalk/15.44.376.0 Mobile Safari/537.36";
        final String defaultUserAgentString = getUserAgent();

        // Check that an attempt to set the default UA string to null or "" has no effect.
        setUserAgent(null);
        assertEquals(defaultUserAgentString, getUserAgent());
        setUserAgent("");
        assertEquals(defaultUserAgentString, getUserAgent());

        // Set a custom UA string, verify that it can be reset back to default.
        setUserAgent(USER_AGENT);
        assertEquals(USER_AGENT, getUserAgent());
        setUserAgent(null);
        assertEquals(defaultUserAgentString, getUserAgent());
    }

    @MediumTest
    @Feature({"UserAgent"})
    public void testUserAgentWithTestServer() throws Throwable {
        final String customUserAgentString = "testUserAgentWithTestServerUserAgent";

        TestWebServer webServer = TestWebServer.start();
        String fileName = null;
        try {
            final String httpPath = "/testUserAgentWithTestServer.html";
            final String url = webServer.setResponse(httpPath, "foo", null);

            setUserAgent(customUserAgentString);
            loadUrlSync(url);

            assertEquals(1, webServer.getRequestCount(httpPath));
            HttpRequest request = webServer.getLastRequest(httpPath);
            Header[] matchingHeaders  = request.getHeaders("User-Agent");
            assertEquals(1, matchingHeaders.length);

            Header header = matchingHeaders[0];
            assertEquals(customUserAgentString, header.getValue());
            assertEquals(customUserAgentString, getUserAgent());
        } finally {
            webServer.shutdown();
        }
    }
}
