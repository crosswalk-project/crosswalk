// Copyright 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;
import android.util.Pair;

import java.util.ArrayList;
import java.util.concurrent.Callable;
import java.util.concurrent.TimeUnit;
import java.util.List;

import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.Feature;
import org.chromium.content_public.browser.LoadUrlParams;
import org.chromium.content.browser.test.util.CallbackHelper;
import org.chromium.content.browser.test.util.TestCallbackHelperContainer.OnPageStartedHelper;
import org.chromium.content.browser.test.util.TestCallbackHelperContainer.OnReceivedErrorHelper;
import org.chromium.net.test.util.TestWebServer;

import org.xwalk.core.xwview.test.util.CommonResources;

/**
 * Tests for the shouldOverrideUrlLoading() method.
 */
public class ShouldOverrideUrlLoadingTest extends XWalkViewTestBase {
    private static final String ABOUT_BLANK_URL = "about:blank";
    private static final String DATA_URL = "data:text/html,<div/>";
    private static final String REDIRECT_TARGET_PATH = "/redirect_target.html";
    private static final String TITLE = "TITLE";
    private TestHelperBridge.ShouldOverrideUrlLoadingHelper mShouldOverrideUrlLoadingHelper;
    private TestWebServer mWebServer;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mShouldOverrideUrlLoadingHelper = mTestHelperBridge.getShouldOverrideUrlLoadingHelper();
        mWebServer = TestWebServer.start();
    }

    @Override
    protected void tearDown() throws Exception {
        mWebServer.shutdown();
        super.tearDown();
    }

    // Since this value is read on the UI thread, it's simpler to set it there too.
    void setShouldOverrideUrlLoadingReturnValueOnUiThread(
            final boolean value) throws Throwable {
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                mShouldOverrideUrlLoadingHelper.setShouldOverrideUrlLoadingReturnValue(value);
            }
        });
    }

    private String getTestPageCommonHeaders() {
        return "<title>" + TITLE + "</title> ";
    }

    private String makeHtmlPageFrom(String headers, String body) {
        return CommonResources.makeHtmlPageFrom(getTestPageCommonHeaders() + headers, body);
    }

    private String getHtmlForPageWithJsAssignLinkTo(String url) {
        return makeHtmlPageFrom("",
                "<img onclick=\"location.href='" + url + "'\" class=\"big\" id=\"link\" />");
    }

    private String getHtmlForPageWithJsReplaceLinkTo(String url) {
        return makeHtmlPageFrom("",
                "<img onclick=\"location.replace('" + url + "');\" class=\"big\" id=\"link\" />");
    }

    private String getHtmlForPageWithMetaRefreshRedirectTo(String url) {
        return makeHtmlPageFrom("<meta http-equiv=\"refresh\" content=\"0;url=" + url + "\" />",
                "<div>Meta refresh redirect</div>");
    }

    private String getHtmlForPageWithJsRedirectTo(String url, String method, int timeout) {
        return makeHtmlPageFrom(
                "<script>" +
                  "function doRedirectAssign() {" +
                    "location.href = '" + url + "';" +
                  "} " +
                  "function doRedirectReplace() {" +
                    "location.replace('" + url + "');" +
                  "} " +
                "</script>",
                String.format("<iframe onLoad=\"setTimeout('doRedirect%s()', %d);\" />",
                    method, timeout));
    }

    private String addPageToTestServer(TestWebServer webServer, String httpPath, String html) {
        List<Pair<String, String>> headers = new ArrayList<Pair<String, String>>();
        headers.add(Pair.create("Content-Type", "text/html"));
        headers.add(Pair.create("Cache-Control", "no-store"));
        return webServer.setResponse(httpPath, html, headers);
    }

    private String createRedirectTargetPage(TestWebServer webServer) {
        return addPageToTestServer(webServer, REDIRECT_TARGET_PATH,
                makeHtmlPageFrom("", "<div>This is the end of the redirect chain</div>"));
    }

    @SmallTest
    @Feature({"XWalkView", "Navigation"})
    public void testNotCalledOnLoadData() throws Throwable {
        loadDataSync(null,
                CommonResources.makeHtmlPageWithSimpleLinkTo(DATA_URL), "text/html", false);

        assertEquals(0, mShouldOverrideUrlLoadingHelper.getCallCount());
    }

    private void waitForNavigationRunnableAndAssertTitleChanged(
            CallbackHelper onPageFinishedHelper,
            Runnable navigationRunnable) throws Exception {
        final int callCount = onPageFinishedHelper.getCallCount();
        final String oldTitle = getTitleOnUiThread();
        getInstrumentation().runOnMainSync(navigationRunnable);
        onPageFinishedHelper.waitForCallback(callCount);
        assertFalse(oldTitle.equals(getTitleOnUiThread()));
    }

    @SmallTest
    @Feature({"XWalkView", "Navigation"})
    public void testCalledOnBackForwardNavigation() throws Throwable {
        final String url = "file:///android_asset/www/index.html";
        final String httpPath = "/test.html";
        final String httpPathOnServer = mWebServer.getResponseUrl(httpPath);
        addPageToTestServer(mWebServer, httpPath,
                CommonResources.makeHtmlPageWithSimpleLinkTo(httpPathOnServer));

        loadUrlSync(httpPathOnServer);
        loadUrlSync(url);
        assertEquals(2, mShouldOverrideUrlLoadingHelper.getCallCount());
        String oldTitle = getTitleOnUiThread();
        goBackSync();
        assertFalse(oldTitle.equals(getTitleOnUiThread()));
        assertEquals(3, mShouldOverrideUrlLoadingHelper.getCallCount());

        oldTitle = getTitleOnUiThread();
        goForwardSync();
        assertFalse(oldTitle.equals(getTitleOnUiThread()));
        assertEquals(4, mShouldOverrideUrlLoadingHelper.getCallCount());
    }

    @SmallTest
    @Feature({"XWalkView", "Navigation"})
    public void testCantBlockLoads() throws Throwable {
        setShouldOverrideUrlLoadingReturnValueOnUiThread(true);

        loadDataSync(null,
                CommonResources.makeHtmlPageWithSimpleLinkTo(getTestPageCommonHeaders(),
                        DATA_URL), "text/html", false);

        assertEquals(TITLE, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"XWalkView", "Navigation"})
    public void testCalledBeforeOnPageStarted() throws Throwable {
        OnPageStartedHelper onPageStartedHelper = mTestHelperBridge.getOnPageStartedHelper();

        loadDataSync(null,
                CommonResources.makeHtmlPageWithSimpleLinkTo(DATA_URL), "text/html", false);

        final int shouldOverrideUrlLoadingCallCount = mShouldOverrideUrlLoadingHelper.getCallCount();
        final int onPageStartedCallCount = onPageStartedHelper.getCallCount();
        setShouldOverrideUrlLoadingReturnValueOnUiThread(true);
        clickOnElementId("link", null);

        mShouldOverrideUrlLoadingHelper.waitForCallback(shouldOverrideUrlLoadingCallCount);
        assertEquals(onPageStartedCallCount, onPageStartedHelper.getCallCount());
    }

    @SmallTest
    @Feature({"XWalkView", "Navigation"})
    public void testDoesNotCauseOnReceivedError() throws Throwable {
        OnReceivedErrorHelper onReceivedErrorHelper = mTestHelperBridge.getOnReceivedErrorHelper();
        final int onReceivedErrorCallCount = onReceivedErrorHelper.getCallCount();

        loadDataSync(null,
                CommonResources.makeHtmlPageWithSimpleLinkTo(DATA_URL), "text/html", false);

        final int shouldOverrideUrlLoadingCallCount = mShouldOverrideUrlLoadingHelper.getCallCount();

        setShouldOverrideUrlLoadingReturnValueOnUiThread(true);

        clickOnElementId("link", null);

        mShouldOverrideUrlLoadingHelper.waitForCallback(shouldOverrideUrlLoadingCallCount);

        setShouldOverrideUrlLoadingReturnValueOnUiThread(false);

        // After we load this URL we're certain that any in-flight callbacks for the previous
        // navigation have been delivered.
        loadUrlSync(ABOUT_BLANK_URL);

        assertEquals(onReceivedErrorCallCount, onReceivedErrorHelper.getCallCount());
    }

    @SmallTest
    @Feature({"XWalkView", "Navigation"})
    public void testNotCalledForAnchorNavigations() throws Throwable {
        doTestNotCalledForAnchorNavigations(false);
    }

    @SmallTest
    @Feature({"XWalkView", "Navigation"})
    public void testNotCalledForAnchorNavigationsWithNonHierarchicalScheme() throws Throwable {
        doTestNotCalledForAnchorNavigations(true);
    }

    private void doTestNotCalledForAnchorNavigations(boolean useLoadData) throws Throwable {
        final String anchorLinkPath = "/anchor_link.html";
        final String anchorLinkUrl = mWebServer.getResponseUrl(anchorLinkPath);
        addPageToTestServer(mWebServer, anchorLinkPath,
                CommonResources.makeHtmlPageWithSimpleLinkTo(anchorLinkUrl + "#anchor"));

        if (useLoadData) {
            loadDataSync(null,
                    CommonResources.makeHtmlPageWithSimpleLinkTo("#anchor"), "text/html", false);
        } else {
            loadUrlSync(anchorLinkUrl);
        }

        final int shouldOverrideUrlLoadingCallCount = mShouldOverrideUrlLoadingHelper.getCallCount();

        clickOnElementId("link", null);

        // After we load this URL we're certain that any in-flight callbacks for the previous
        // navigation have been delivered.
        loadUrlSync(ABOUT_BLANK_URL);

        assertEquals(shouldOverrideUrlLoadingCallCount,
                mShouldOverrideUrlLoadingHelper.getCallCount());
    }

    @SmallTest
    @Feature({"XWalkView", "Navigation"})
    public void testCalledWhenLinkClicked() throws Throwable {
        // We can't go to about:blank from here because we'd get a cross-origin error.
        loadDataSync(null,
                CommonResources.makeHtmlPageWithSimpleLinkTo(DATA_URL), "text/html", false);

        int callCount = mShouldOverrideUrlLoadingHelper.getCallCount();

        clickOnElementId("link", null);

        mShouldOverrideUrlLoadingHelper.waitForCallback(callCount);
    }

    @SmallTest
    @Feature({"XWalkView", "Navigation"})
    public void testCalledWhenSelfLinkClicked() throws Throwable {
        final String httpPath = "/page_with_link_to_self.html";
        final String httpPathOnServer = mWebServer.getResponseUrl(httpPath);
        addPageToTestServer(mWebServer, httpPath,
                CommonResources.makeHtmlPageWithSimpleLinkTo(httpPathOnServer));

        loadUrlSync(httpPathOnServer);

        int callCount = mShouldOverrideUrlLoadingHelper.getCallCount();

        clickOnElementId("link", null);

        mShouldOverrideUrlLoadingHelper.waitForCallback(callCount);
        assertEquals(httpPathOnServer,
                mShouldOverrideUrlLoadingHelper.getShouldOverrideUrlLoadingUrl());
    }

    @SmallTest
    @Feature({"XWalkView", "Navigation"})
    public void testCalledWhenNavigatingFromJavaScriptUsingAssign()
            throws Throwable {
        final String redirectTargetUrl = createRedirectTargetPage(mWebServer);
        loadDataSync(null,
                getHtmlForPageWithJsAssignLinkTo(redirectTargetUrl), "text/html", false);

        int callCount = mShouldOverrideUrlLoadingHelper.getCallCount();

        clickOnElementId("link", null);

        mShouldOverrideUrlLoadingHelper.waitForCallback(callCount);
    }

    @SmallTest
    @Feature({"XWalkView", "Navigation"})
    public void testCalledWhenNavigatingFromJavaScriptUsingReplace()
            throws Throwable {
        final String redirectTargetUrl = createRedirectTargetPage(mWebServer);
        loadDataSync(null,
                getHtmlForPageWithJsReplaceLinkTo(redirectTargetUrl), "text/html", false);

        int callCount = mShouldOverrideUrlLoadingHelper.getCallCount();
        clickOnElementId("link", null);
        mShouldOverrideUrlLoadingHelper.waitForCallback(callCount);
    }

    @SmallTest
    @Feature({"XWalkView", "Navigation"})
    public void testPassesCorrectUrl() throws Throwable {
        final String redirectTargetUrl = createRedirectTargetPage(mWebServer);
        loadDataSync(null,
                CommonResources.makeHtmlPageWithSimpleLinkTo(redirectTargetUrl), "text/html", false);

        int callCount = mShouldOverrideUrlLoadingHelper.getCallCount();
        clickOnElementId("link", null);
        mShouldOverrideUrlLoadingHelper.waitForCallback(callCount);
        assertEquals(redirectTargetUrl,
                mShouldOverrideUrlLoadingHelper.getShouldOverrideUrlLoadingUrl());
    }

    // This test case is not stable, disable it temporarily.
    // @SmallTest
    // @Feature({"XWalkView", "Navigation"})
    @DisabledTest
    public void testCanIgnoreLoading() throws Throwable {
        final String redirectTargetUrl = createRedirectTargetPage(mWebServer);
        final String pageWithLinkToIgnorePath = "/page_with_link_to_ignore.html";
        final String pageWithLinkToIgnoreUrl = addPageToTestServer(mWebServer,
                pageWithLinkToIgnorePath, CommonResources.makeHtmlPageWithSimpleLinkTo(redirectTargetUrl));
        final String synchronizationPath = "/sync.html";
        final String synchronizationUrl = addPageToTestServer(mWebServer, synchronizationPath,
                CommonResources.makeHtmlPageWithSimpleLinkTo(redirectTargetUrl));

        loadUrlSync(pageWithLinkToIgnoreUrl);

        setShouldOverrideUrlLoadingReturnValueOnUiThread(true);

        int callCount = mShouldOverrideUrlLoadingHelper.getCallCount();
        clickOnElementId("link", null);
        // Some time around here true should be returned from the shouldOverrideUrlLoading
        // callback causing the navigation caused by calling clickOnElementId to be ignored.
        // We validate this by checking which pages were loaded on the server.
        mShouldOverrideUrlLoadingHelper.waitForCallback(callCount);

        setShouldOverrideUrlLoadingReturnValueOnUiThread(false);

        loadUrlSync(synchronizationUrl);

        assertEquals(1, mWebServer.getRequestCount(pageWithLinkToIgnorePath));
        assertEquals(1, mWebServer.getRequestCount(synchronizationPath));
        assertEquals(0, mWebServer.getRequestCount(REDIRECT_TARGET_PATH));
    }

    @SmallTest
    @Feature({"XWalkView", "Navigation"})
    public void testCalledForDataUrl() throws Throwable {
        final String dataUrl =
                "data:text/html;base64," +
                "PGh0bWw+PGhlYWQ+PHRpdGxlPmRhdGFVcmxUZXN0QmFzZTY0PC90aXRsZT48" +
                "L2hlYWQ+PC9odG1sPg==";
        loadDataSync(null,
                CommonResources.makeHtmlPageWithSimpleLinkTo(dataUrl), "text/html", false);

        int callCount = mShouldOverrideUrlLoadingHelper.getCallCount();
        clickOnElementId("link", null);

        mShouldOverrideUrlLoadingHelper.waitForCallback(callCount);
        assertTrue("Expected URL that starts with 'data:' but got: <" +
                mShouldOverrideUrlLoadingHelper.getShouldOverrideUrlLoadingUrl() + "> instead.",
                mShouldOverrideUrlLoadingHelper.getShouldOverrideUrlLoadingUrl().startsWith(
                        "data:"));
    }

    @SmallTest
    @Feature({"XWalkView", "Navigation"})
    public void testCalledForUnsupportedSchemes() throws Throwable {
        final String unsupportedSchemeUrl = "foobar://resource/1";
        loadDataSync(null,
                CommonResources.makeHtmlPageWithSimpleLinkTo(unsupportedSchemeUrl), "text/html",
                        false);

        int callCount = mShouldOverrideUrlLoadingHelper.getCallCount();
        clickOnElementId("link", null);

        mShouldOverrideUrlLoadingHelper.waitForCallback(callCount);
        assertEquals(unsupportedSchemeUrl,
                mShouldOverrideUrlLoadingHelper.getShouldOverrideUrlLoadingUrl());
    }

    @SmallTest
    @Feature({"XWalkView", "Navigation"})
    public void testCalledForPostNavigations() throws Throwable {
        // The reason POST requests are excluded is BUG 155250.
        final String redirectTargetUrl = createRedirectTargetPage(mWebServer);
        final String postLinkUrl = addPageToTestServer(mWebServer, "/page_with_post_link.html",
                CommonResources.makeHtmlPageWithSimplePostFormTo(redirectTargetUrl));

        loadUrlSync(postLinkUrl);

        final int shouldOverrideUrlLoadingCallCount =
            mShouldOverrideUrlLoadingHelper.getCallCount();

        assertEquals(0, mWebServer.getRequestCount(REDIRECT_TARGET_PATH));
        clickOnElementId("link", null);

        // Wait for the target URL to be fetched from the server.
        pollOnUiThread(new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return mWebServer.getRequestCount(REDIRECT_TARGET_PATH) == 1;
            }
        });

        // Since the targetURL was loaded from the test server it means all processing related
        // to dispatching a shouldOverrideUrlLoading callback had finished and checking the call
        // is stable.
        assertEquals(shouldOverrideUrlLoadingCallCount + 1,
                mShouldOverrideUrlLoadingHelper.getCallCount());
    }

    @SmallTest
    @Feature({"XWalkView", "Navigation"})
    public void testCalledFor302AfterPostNavigations() throws Throwable {
        // The reason POST requests are excluded is BUG 155250.
        final String redirectTargetUrl = createRedirectTargetPage(mWebServer);
        final String postToGetRedirectUrl = mWebServer.setRedirect("/302.html", redirectTargetUrl);
        final String postLinkUrl = addPageToTestServer(mWebServer, "/page_with_post_link.html",
                CommonResources.makeHtmlPageWithSimplePostFormTo(postToGetRedirectUrl));

        loadUrlSync(postLinkUrl);

        final int shouldOverrideUrlLoadingCallCount = mShouldOverrideUrlLoadingHelper.getCallCount();

        clickOnElementId("link", null);

        mShouldOverrideUrlLoadingHelper.waitForCallback(shouldOverrideUrlLoadingCallCount);

        // Wait for the target URL to be fetched from the server.
        pollOnUiThread(new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return mWebServer.getRequestCount(REDIRECT_TARGET_PATH) == 1;
            }
        });

        assertEquals(redirectTargetUrl,
                mShouldOverrideUrlLoadingHelper.getShouldOverrideUrlLoadingUrl());
    }

    @SmallTest
    @Feature({"XWalkView", "Navigation"})
    public void testCalledForIframeHttpNavigations() throws Throwable {
        final String iframeRedirectTargetUrl = createRedirectTargetPage(mWebServer);
        final String iframeRedirectUrl = mWebServer.setRedirect("/302.html", iframeRedirectTargetUrl);
        final String pageWithIframeUrl = addPageToTestServer(mWebServer, "/iframe_intercept.html",
                makeHtmlPageFrom("", "<iframe src=\"" + iframeRedirectUrl + "\" />"));

        assertEquals(0, mWebServer.getRequestCount(REDIRECT_TARGET_PATH));
        loadUrlSync(pageWithIframeUrl);

        // Wait for the redirect target URL to be fetched from the server.
        pollOnUiThread(new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return mWebServer.getRequestCount(REDIRECT_TARGET_PATH) == 1;
            }
        });

        assertEquals(1, mShouldOverrideUrlLoadingHelper.getCallCount());
    }

    /**
     * Worker method for the various redirect tests.
     *
     * Calling this will first load the redirect URL built from redirectFilePath, query and
     * locationFilePath and assert that we get a override callback for the destination.
     * The second part of the test loads a page that contains a link which points at the redirect
     * URL. We expect two callbacks - one for the redirect link and another for the destination.
     */
    private void doTestCalledOnRedirect(TestWebServer webServer,
            String redirectUrl, String redirectTarget) throws Throwable {
        final String pageWithLinkToRedirectUrl = addPageToTestServer(webServer,
                "/page_with_link_to_redirect.html",
                         CommonResources.makeHtmlPageWithSimpleLinkTo(redirectUrl));
        int directLoadCallCount = mShouldOverrideUrlLoadingHelper.getCallCount();
        loadUrlSync(redirectUrl);

        mShouldOverrideUrlLoadingHelper.waitForCallback(directLoadCallCount, 2);
        assertEquals(redirectTarget,
                mShouldOverrideUrlLoadingHelper.getShouldOverrideUrlLoadingUrl());

        int indirectLoadCallCount = mShouldOverrideUrlLoadingHelper.getCallCount();
        loadUrlSync(pageWithLinkToRedirectUrl);

        clickOnElementId("link", null);

        mShouldOverrideUrlLoadingHelper.waitForCallback(indirectLoadCallCount, 3);
        assertEquals(redirectTarget,
                mShouldOverrideUrlLoadingHelper.getShouldOverrideUrlLoadingUrl());
        assertEquals(redirectUrl,
                mShouldOverrideUrlLoadingHelper.getPreviousShouldOverrideUrlLoadingUrl());
    }

    @SmallTest
    @Feature({"XWalkView", "Navigation"})
    public void testCalledOn302Redirect() throws Throwable {
        final String redirectTargetUrl = createRedirectTargetPage(mWebServer);
        final String redirectUrl = mWebServer.setRedirect("/302.html", redirectTargetUrl);

        doTestCalledOnRedirect(mWebServer, redirectUrl, redirectTargetUrl);
    }

    @SmallTest
    @Feature({"XWalkView", "Navigation"})
    public void testCalledOnMetaRefreshRedirect() throws Throwable {
        final String redirectTargetUrl = createRedirectTargetPage(mWebServer);
        final String redirectUrl = addPageToTestServer(mWebServer, "/meta_refresh.html",
                getHtmlForPageWithMetaRefreshRedirectTo(redirectTargetUrl));
        doTestCalledOnRedirect(mWebServer, redirectUrl, redirectTargetUrl);
    }


    @SmallTest
    @Feature({"XWalkView", "Navigation"})
    public void testCalledOnJavaScriptLocationImmediateAssignRedirect()
            throws Throwable {
        final String redirectTargetUrl = createRedirectTargetPage(mWebServer);
        final String redirectUrl = addPageToTestServer(mWebServer, "/js_immediate_assign.html",
                getHtmlForPageWithJsRedirectTo(redirectTargetUrl, "Assign", 0));
        doTestCalledOnRedirect(mWebServer, redirectUrl, redirectTargetUrl);
    }

    @SmallTest
    @Feature({"XWalkView", "Navigation"})
    public void testCalledOnJavaScriptLocationImmediateReplaceRedirect()
            throws Throwable {
        final String redirectTargetUrl = createRedirectTargetPage(mWebServer);
        final String redirectUrl = addPageToTestServer(mWebServer, "/js_immediate_replace.html",
                getHtmlForPageWithJsRedirectTo(redirectTargetUrl, "Replace", 0));
        doTestCalledOnRedirect(mWebServer, redirectUrl, redirectTargetUrl);
    }

    @SmallTest
    @Feature({"XWalkView", "Navigation"})
    public void testCalledOnJavaScriptLocationDelayedAssignRedirect()
            throws Throwable {
        final String redirectTargetUrl = createRedirectTargetPage(mWebServer);
        final String redirectUrl = addPageToTestServer(mWebServer, "/js_delayed_assign.html",
                getHtmlForPageWithJsRedirectTo(redirectTargetUrl, "Assign", 100));
        doTestCalledOnRedirect(mWebServer, redirectUrl, redirectTargetUrl);
    }

    @SmallTest
    @Feature({"XWalkView", "Navigation"})
    public void testCalledOnJavaScriptLocationDelayedReplaceRedirect()
            throws Throwable {
        final String redirectTargetUrl = createRedirectTargetPage(mWebServer);
        final String redirectUrl = addPageToTestServer(mWebServer, "/js_delayed_replace.html",
                getHtmlForPageWithJsRedirectTo(redirectTargetUrl, "Replace", 100));
        doTestCalledOnRedirect(mWebServer, redirectUrl, redirectTargetUrl);
    }

    @SmallTest
    @Feature({"XWalkView", "Navigation"})
    public void testDoubleNavigateDoesNotSuppressInitialNavigate() throws Throwable {
        final String jsUrl = "javascript:try{console.log('processed js loadUrl');}catch(e){};";

        // Do a double navigagtion, the second being an effective no-op, in quick succession (i.e.
        // without yielding the main thread inbetween).
        loadDataSync(null,
                CommonResources.makeHtmlPageWithSimpleLinkTo(DATA_URL), "text/html", false);
        loadJavaScriptUrl(jsUrl);
        assertEquals(0, mShouldOverrideUrlLoadingHelper.getCallCount());
    }
}
