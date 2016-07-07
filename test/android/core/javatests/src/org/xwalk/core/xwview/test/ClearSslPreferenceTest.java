// Copyright 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.Feature;
import org.chromium.content.browser.test.util.CallbackHelper;
import org.chromium.net.test.util.TestWebServer;

public class ClearSslPreferenceTest extends XWalkViewTestBase {
    private TestWebServer mWebServer;

    @Override
    public void setUp() throws Exception {
        super.setUp();

        mWebServer = TestWebServer.startSsl();
    }

    @Override
    protected void tearDown() throws Exception {
        mWebServer.shutdown();

        super.tearDown();
    }

    // @Feature({"ClearSslPreference"})
    // @SmallTest
    // If the user allows the ssl error, the same ssl error will not trigger
    // the onReceivedSslError callback; If the user denies it, the same ssl
    // error will still trigger the onReceivedSslError callback.
    // For SSL error, if user allows it, the related host and error will be recorded.
    // All future communications with same host and error, will accept any SSL certificate
    // even if not valid. We deny invalid requests with some serious ssl errors that
    // this case will be failed, so disbaled it.
    @DisabledTest
    public void testSslPreferences() throws Throwable {
        final String pagePath = "/hello.html";
        final String pageUrl =
                mWebServer.setResponse(pagePath, "<html><body>hello world</body></html>", null);
        final CallbackHelper onReceivedSslErrorHelper =
                mTestHelperBridge.getOnReceivedSslErrorHelper();
        int onSslErrorCallCount = onReceivedSslErrorHelper.getCallCount();

        assertNull(getCertificateOnUiThread());
        loadUrlSync(pageUrl);
        assertNotNull(getCertificateOnUiThread());

        assertEquals(onSslErrorCallCount + 1, onReceivedSslErrorHelper.getCallCount());
        assertEquals(1, mWebServer.getRequestCount(pagePath));

        // Now load the page again. This time, we expect no ssl error, because
        // user's decision should be remembered.
        onSslErrorCallCount = onReceivedSslErrorHelper.getCallCount();
        loadUrlSync(pageUrl);
        assertEquals(onSslErrorCallCount, onReceivedSslErrorHelper.getCallCount());

        // Now clear the ssl preferences then load the same url again. Expect to see
        // onReceivedSslError getting called again.
        clearSslPreferences();
        onSslErrorCallCount = onReceivedSslErrorHelper.getCallCount();
        loadUrlSync(pageUrl);
        assertEquals(onSslErrorCallCount + 1, onReceivedSslErrorHelper.getCallCount());

        // Now clear the stored decisions and tell the client to deny ssl errors.
        clearSslPreferences();
        setAllowSslError(false);
        onSslErrorCallCount = onReceivedSslErrorHelper.getCallCount();
        loadUrlSync(pageUrl);
        assertEquals(onSslErrorCallCount + 1, onReceivedSslErrorHelper.getCallCount());

        // Now load the same page again. This time, we still expect onReceivedSslError,
        // because we only remember user's decision if it is "allow".
        onSslErrorCallCount = onReceivedSslErrorHelper.getCallCount();
        loadUrlSync(pageUrl);
        assertNotNull(getCertificateOnUiThread());
        assertEquals(onSslErrorCallCount + 1, onReceivedSslErrorHelper.getCallCount());
    }

    @Feature({"SslError"})
    @SmallTest
    public void testDeniedRequestForSslError() throws Throwable {
        final String pagePath = "/hello.html";
        final String pageUrl =
                mWebServer.setResponse(pagePath, "<html><body>hello world</body></html>", null);
        final CallbackHelper onReceivedSslErrorHelper =
                mTestHelperBridge.getOnReceivedSslErrorHelper();
        int onSslErrorCallCount = onReceivedSslErrorHelper.getCallCount();

        assertNull(getCertificateOnUiThread());
        loadUrlSync(pageUrl);
        // Request was denied for ERR_CERT_COMMON_NAME_INVALID, certificate was
        // also NULL.
        assertNull(getCertificateOnUiThread());
    }
}
