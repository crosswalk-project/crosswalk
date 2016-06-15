// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.xwview.test;

import android.graphics.Bitmap;
import android.test.suitebuilder.annotation.SmallTest;
import android.util.Pair;
import android.webkit.ValueCallback;
import android.webkit.WebResourceResponse;

import java.util.concurrent.atomic.AtomicReference;
import java.util.concurrent.Callable;

import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.Feature;
import org.chromium.content.browser.test.util.CallbackHelper;
import org.chromium.content.browser.test.util.Criteria;
import org.chromium.content.browser.test.util.CriteriaHelper;
import org.chromium.net.test.util.TestWebServer;
import org.xwalk.core.internal.XWalkViewInternal;
import org.xwalk.core.internal.XWalkClient;
import org.xwalk.core.internal.XWalkSettingsInternal;
import org.xwalk.core.internal.XWalkWebChromeClient;
import org.xwalk.core.internal.xwview.test.util.CommonResources;

/**
 * Test suite for setAppCacheEnabled().
 */
public class SetAppCacheEnabledTest extends XWalkViewInternalTestBase {
    private static final long TEST_TIMEOUT = 20000L;
    private static final int CHECK_INTERVAL = 100;
    private TestHelperBridge mContentClient;
    private XWalkSettingsInternal mSettings;

    static class ManifestTestHelper {
        private final TestWebServer mWebServer;
        private final String mHtmlPath;
        private final String mHtmlUrl;
        private final String mManifestPath;

        ManifestTestHelper(TestWebServer webServer, String htmlPageName, String manifestName) {
            mWebServer = webServer;
            mHtmlPath = "/" + htmlPageName;
            mHtmlUrl = webServer.setResponse(
                    mHtmlPath, "<html manifest=\"" + manifestName + "\"></html>", null);
            mManifestPath = "/" + manifestName;
            webServer.setResponse(
                    mManifestPath,
                    "CACHE MANIFEST",
                    CommonResources.getContentTypeAndCacheHeaders("text/cache-manifest", false));
        }

        String getHtmlPath() {
            return mHtmlPath;
        }

        String getHtmlUrl() {
            return mHtmlUrl;
        }

        String getManifestPath() {
            return mManifestPath;
        }

        int waitUntilHtmlIsRequested(final int initialRequestCount) throws InterruptedException {
            return waitUntilResourceIsRequested(mHtmlPath, initialRequestCount);
        }

        int waitUntilManifestIsRequested(final int initialRequestCount)
                throws InterruptedException {
            return waitUntilResourceIsRequested(mManifestPath, initialRequestCount);
        }

        private int waitUntilResourceIsRequested(
                final String path, final int initialRequestCount) throws InterruptedException {
            CriteriaHelper.pollInstrumentationThread(new Criteria() {
                @Override
                public boolean isSatisfied() {
                    return mWebServer.getRequestCount(path) > initialRequestCount;
                }
            }, TEST_TIMEOUT, CHECK_INTERVAL);
            return mWebServer.getRequestCount(path);
        }
    }

    private XWalkSettingsInternal getXWalkSettings(final XWalkViewInternal view) {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mSettings = view.getSettings();
            }
        });
        return mSettings;
    }

    @SmallTest
    @Feature({"XWalkViewInternal", "Preferences", "AppCache"})
    public void testAppCache() throws Throwable {
        final TestHelperBridge helperBridge =
                new TestHelperBridge();
        mContentClient = helperBridge;
        final XWalkViewInternalTestBase.TestXWalkUIClientInternalBase uiClient =
                new XWalkViewInternalTestBase.TestXWalkUIClientInternalBase(helperBridge);
        final XWalkViewInternalTestBase.TestXWalkResourceClientBase resourceClient =
                new XWalkViewInternalTestBase.TestXWalkResourceClientBase(helperBridge);
        final XWalkViewInternal xWalkViewInternal =
                createXWalkViewContainerOnMainSync(getActivity(), uiClient,
                        resourceClient);

        final XWalkSettingsInternal settings = getXWalkSettings(xWalkViewInternal);
        settings.setJavaScriptEnabled(true);
        settings.setAppCacheEnabled(false);

        TestWebServer webServer = TestWebServer.start();
        try {
            ManifestTestHelper helper = new ManifestTestHelper(
                    webServer, "testAppCache.html", "appcache.manifest");
            loadUrlSyncByContent(
                    xWalkViewInternal,
                    mContentClient,
                    helper.getHtmlUrl());
            helper.waitUntilHtmlIsRequested(0);
            // Unfortunately, there is no other good way of verifying that AppCache is
            // disabled, other than checking that it didn't try to fetch the manifest.
            Thread.sleep(1000);
            assertEquals(0, webServer.getRequestCount(helper.getManifestPath()));
            // Enables AppCache. Use the default path if app cache path isn't set.
            settings.setAppCacheEnabled(true);
            loadUrlSyncByContent(
                    xWalkViewInternal,
                    mContentClient,
                    helper.getHtmlUrl());
            helper.waitUntilManifestIsRequested(0);
        } finally {
            webServer.shutdown();
        }
    }

    /*
     * @SmallTest
     * @Feature({"XWalkViewInternal", "Preferences", "AppCache"})
     * This test is flaky but the root cause is not found yet. See crbug.com/171765.
     */
    @DisabledTest
    public void testAppCacheWithTwoViews() throws Throwable {
        // We don't use the test helper here, because making sure that AppCache
        // is disabled takes a lot of time, so running through the usual drill
        // will take about 20 seconds.
        ViewPair views = createViews();

        XWalkSettingsInternal settings0 = getXWalkSettingsOnUiThreadByContent(
                views.getView0());
        settings0.setJavaScriptEnabled(true);
        settings0.setAppCachePath("whatever");
        settings0.setAppCacheEnabled(true);
        XWalkSettingsInternal settings1 = getXWalkSettingsOnUiThreadByContent(
                views.getView1());
        settings1.setJavaScriptEnabled(true);
        // AppCachePath setting is global, no need to set it for the second view.
        settings1.setAppCacheEnabled(true);

        TestWebServer webServer = TestWebServer.start();
        try {
            ManifestTestHelper helper0 = new ManifestTestHelper(
                    webServer, "testAppCache_0.html", "appcache.manifest_0");
            mContentClient = views.getClient0();
            loadUrlSyncByContent(
                    views.getView0(),
                    mContentClient,
                    helper0.getHtmlUrl());
            int manifestRequests0 = helper0.waitUntilManifestIsRequested(0);
            ManifestTestHelper helper1 = new ManifestTestHelper(
                    webServer, "testAppCache_1.html", "appcache.manifest_1");
            mContentClient = views.getClient1();
            loadUrlSyncByContent(
                    views.getView1(),
                    mContentClient,
                    helper1.getHtmlUrl());
            helper1.waitUntilManifestIsRequested(0);
            settings1.setAppCacheEnabled(false);
            mContentClient = views.getClient0();
            loadUrlSyncByContent(
                    views.getView0(),
                    mContentClient,
                    helper0.getHtmlUrl());
            helper0.waitUntilManifestIsRequested(manifestRequests0);
            final int prevManifestRequestCount =
                    webServer.getRequestCount(helper1.getManifestPath());
            int htmlRequests1 = webServer.getRequestCount(helper1.getHtmlPath());
            mContentClient = views.getClient1();
            loadUrlSyncByContent(
                    views.getView1(),
                    mContentClient,
                    helper1.getHtmlUrl());
            helper1.waitUntilHtmlIsRequested(htmlRequests1);
            // Unfortunately, there is no other good way of verifying that AppCache is
            // disabled, other than checking that it didn't try to fetch the manifest.
            Thread.sleep(1000);
            assertEquals(
                    prevManifestRequestCount, webServer.getRequestCount(helper1.getManifestPath()));
        } finally {
            webServer.shutdown();
        }
    }
}
