// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;
import android.util.Pair;
import android.webkit.WebResourceResponse;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CountDownLatch;

import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.Feature;
import org.chromium.base.test.util.TestFileUtil;
import org.chromium.content.browser.test.util.CallbackHelper;
import org.chromium.content.browser.test.util.TestCallbackHelperContainer.OnReceivedErrorHelper;
import org.chromium.net.test.util.TestWebServer;

import org.xwalk.core.XWalkView;
import org.xwalk.core.xwview.test.TestContentProvider;
import org.xwalk.core.xwview.test.util.CommonResources;

/**
 * Test case for XWalkResourceClient.shouldInterceptRequest callback
 *
 * Note the major part of this file is migrated from android_webview/.
 */
public class ShouldInterceptLoadRequestTest extends XWalkViewTestBase {

    /**
     * Customized XWalkResourceClient implementation for shouldInterceptRequest
     */
    private class TestXWalkResourceClient1 extends XWalkViewTestBase.TestXWalkResourceClient {
        @Override
        public WebResourceResponse shouldInterceptLoadRequest(XWalkView view, String url) {
            return mTestHelperBridge.shouldInterceptLoadRequest(url);
        }

    }

    private String addPageToTestServer(TestWebServer webServer, String httpPath, String html) {
        List<Pair<String, String>> headers = new ArrayList<Pair<String, String>>();
        headers.add(Pair.create("Content-Type", "text/html"));
        headers.add(Pair.create("Cache-Control", "no-store"));
        return webServer.setResponse(httpPath, html, headers);
    }

    private String addAboutPageToTestServer(TestWebServer webServer) {
        return addPageToTestServer(webServer, "/" + CommonResources.ABOUT_FILENAME,
                CommonResources.ABOUT_HTML);
    }

    private WebResourceResponse stringToWebResourceResponse(String input) throws Throwable {
        final String mimeType = "text/html";
        final String encoding = "UTF-8";

        return new WebResourceResponse(
                mimeType, encoding, new ByteArrayInputStream(input.getBytes(encoding)));
    }

    private TestWebServer mWebServer;
    private TestXWalkResourceClient1 mTestXWalkResourceClient;
    private TestHelperBridge.ShouldInterceptLoadRequestHelper mShouldInterceptLoadRequestHelper;
    private TestHelperBridge.OnLoadStartedHelper mOnLoadStartedHelper;

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        setXWalkClient(new XWalkViewTestBase.TestXWalkClient());
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mTestXWalkResourceClient = new TestXWalkResourceClient1();
                getXWalkView().setResourceClient(mTestXWalkResourceClient);
                mShouldInterceptLoadRequestHelper = mTestHelperBridge.getShouldInterceptLoadRequestHelper();
                mOnLoadStartedHelper = mTestHelperBridge.getOnLoadStartedHelper();
            }
        });

        mWebServer = new TestWebServer(false);
    }

    @Override
    protected void tearDown() throws Exception {
        mWebServer.shutdown();
        super.tearDown();
    }

    @SmallTest
    @Feature({"ShouldInterceptLoadRequest"})
    public void testCalledWithCorrectUrl() throws Throwable {
        final String aboutPageUrl = addAboutPageToTestServer(mWebServer);

        int callCount = mShouldInterceptLoadRequestHelper.getCallCount();
        int onPageFinishedCallCount = mTestHelperBridge.getOnPageFinishedHelper().getCallCount();

        loadUrlAsync(aboutPageUrl);

        mShouldInterceptLoadRequestHelper.waitForCallback(callCount);
        assertEquals(1, mShouldInterceptLoadRequestHelper.getUrls().size());
        assertEquals(aboutPageUrl,
                mShouldInterceptLoadRequestHelper.getUrls().get(0));

        mTestHelperBridge.getOnPageFinishedHelper().waitForCallback(onPageFinishedCallCount);
        assertEquals(CommonResources.ABOUT_TITLE, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"ShouldInterceptLoadRequest"})
    public void testOnLoadResourceCalledWithCorrectUrl() throws Throwable {
        final String aboutPageUrl = addAboutPageToTestServer(mWebServer);
        int callCount = mOnLoadStartedHelper.getCallCount();

        loadUrlAsync(aboutPageUrl);

        mOnLoadStartedHelper.waitForCallback(callCount);
        assertEquals(aboutPageUrl, mOnLoadStartedHelper.getUrl());
    }

    @SmallTest
    @Feature({"ShouldInterceptLoadRequest"})
    public void testDoesNotCrashOnInvalidData() throws Throwable {
        final String aboutPageUrl = addAboutPageToTestServer(mWebServer);

        mShouldInterceptLoadRequestHelper.setReturnValue(
                new WebResourceResponse("text/html", "UTF-8", null));
        int callCount = mShouldInterceptLoadRequestHelper.getCallCount();
        loadUrlAsync(aboutPageUrl);
        mShouldInterceptLoadRequestHelper.waitForCallback(callCount);

        mShouldInterceptLoadRequestHelper.setReturnValue(
                new WebResourceResponse(null, null, new ByteArrayInputStream(new byte[0])));
        callCount = mShouldInterceptLoadRequestHelper.getCallCount();
        loadUrlAsync(aboutPageUrl);
        mShouldInterceptLoadRequestHelper.waitForCallback(callCount);

        mShouldInterceptLoadRequestHelper.setReturnValue(
                new WebResourceResponse(null, null, null));
        callCount = mShouldInterceptLoadRequestHelper.getCallCount();
        loadUrlAsync(aboutPageUrl);
        mShouldInterceptLoadRequestHelper.waitForCallback(callCount);
    }

    private static class EmptyInputStream extends InputStream {
        @Override
        public int available() {
            return 0;
        }

        @Override
        public int read() throws IOException {
            return -1;
        }

        @Override
        public int read(byte b[]) throws IOException {
            return -1;
        }

        @Override
        public int read(byte b[], int off, int len) throws IOException {
            return -1;
        }

        @Override
        public long skip(long n) throws IOException {
            if (n < 0)
                throw new IOException("skipping negative number of bytes");
            return 0;
        }
    }

    @SmallTest
    @Feature({"ShouldInterceptLoadRequest"})
    public void testDoesNotCrashOnEmptyStream() throws Throwable {
        final String aboutPageUrl = addAboutPageToTestServer(mWebServer);

        mShouldInterceptLoadRequestHelper.setReturnValue(
                new WebResourceResponse("text/html", "UTF-8", new EmptyInputStream()));
        int shouldInterceptRequestCallCount = mShouldInterceptLoadRequestHelper.getCallCount();
        int onPageFinishedCallCount = mTestHelperBridge.getOnPageFinishedHelper().getCallCount();

        loadUrlAsync(aboutPageUrl);

        mShouldInterceptLoadRequestHelper.waitForCallback(shouldInterceptRequestCallCount);
        mTestHelperBridge.getOnPageFinishedHelper().waitForCallback(onPageFinishedCallCount);
    }

    private static class SlowWebResourceResponse extends WebResourceResponse {
        private CallbackHelper mReadStartedCallbackHelper = new CallbackHelper();
        private CountDownLatch mLatch = new CountDownLatch(1);

        public SlowWebResourceResponse(String mimeType, String encoding, InputStream data) {
            super(mimeType, encoding, data);
        }

        @Override
        public InputStream getData() {
            mReadStartedCallbackHelper.notifyCalled();
            try {
                mLatch.await();
            } catch (InterruptedException e) {
                // ignore
            }
            return super.getData();
        }

        public void unblockReads() {
            mLatch.countDown();
        }

        public CallbackHelper getReadStartedCallbackHelper() {
            return mReadStartedCallbackHelper;
        }
    }

    @SmallTest
    @Feature({"ShouldInterceptLoadRequest"})
    public void testHttpStatusField() throws Throwable {
        final String syncGetUrl = mWebServer.getResponseUrl("/intercept_me");
        final String syncGetJs =
            "(function() {" +
            "  var xhr = new XMLHttpRequest();" +
            "  xhr.open('GET', '" + syncGetUrl + "', false);" +
            "  xhr.send(null);" +
            "  console.info('xhr.status = ' + xhr.status);" +
            "  return xhr.status;" +
            "})();";

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getXWalkView().getSettings().setJavaScriptEnabled(true);
            }
        });

        final String aboutPageUrl = addAboutPageToTestServer(mWebServer);
        loadUrlSync(aboutPageUrl);

        mShouldInterceptLoadRequestHelper.setReturnValue(
                new WebResourceResponse("text/html", "UTF-8", null));
        assertEquals("404", executeJavaScriptAndWaitForResult(syncGetJs));

        mShouldInterceptLoadRequestHelper.setReturnValue(
                new WebResourceResponse("text/html", "UTF-8", new EmptyInputStream()));
        assertEquals("200", executeJavaScriptAndWaitForResult(syncGetJs));
    }


    private String makePageWithTitle(String title) {
        return CommonResources.makeHtmlPageFrom("<title>" + title + "</title>",
                "<div> The title is: " + title + " </div>");
    }

    @SmallTest
    @Feature({"ShouldInterceptLoadRequest"})
    public void testCanInterceptMainFrame() throws Throwable {
        final String expectedTitle = "testShouldInterceptLoadRequestCanInterceptMainFrame";
        final String expectedPage = makePageWithTitle(expectedTitle);

        mShouldInterceptLoadRequestHelper.setReturnValue(
                stringToWebResourceResponse(expectedPage));

        final String aboutPageUrl = addAboutPageToTestServer(mWebServer);

        loadUrlSync(aboutPageUrl);

        assertEquals(expectedTitle, getTitleOnUiThread());
        assertEquals(0, mWebServer.getRequestCount("/" + CommonResources.ABOUT_FILENAME));
    }

    @SmallTest
    @Feature({"ShouldInterceptLoadRequest"})
    public void testDoesNotChangeReportedUrl() throws Throwable {
        mShouldInterceptLoadRequestHelper.setReturnValue(
                stringToWebResourceResponse(makePageWithTitle("some title")));

        final String aboutPageUrl = addAboutPageToTestServer(mWebServer);

        loadUrlSync(aboutPageUrl);

        assertEquals(aboutPageUrl, mTestHelperBridge.getOnPageFinishedHelper().getUrl());
        assertEquals(aboutPageUrl, mTestHelperBridge.getOnPageStartedHelper().getUrl());
    }

    @SmallTest
    @Feature({"ShouldInterceptLoadRequest"})
    public void testNullInputStreamCausesErrorForMainFrame() throws Throwable {
        final OnReceivedErrorHelper onReceivedErrorHelper = mTestHelperBridge.getOnReceivedErrorHelper();
        mShouldInterceptLoadRequestHelper.setReturnValue(
                new WebResourceResponse("text/html", "UTF-8", null));

        final String aboutPageUrl = addAboutPageToTestServer(mWebServer);
        final int callCount = onReceivedErrorHelper.getCallCount();
        loadUrlAsync(aboutPageUrl);
        onReceivedErrorHelper.waitForCallback(callCount);
        assertEquals(0, mWebServer.getRequestCount("/" + CommonResources.ABOUT_FILENAME));
    }

    @SmallTest
    @Feature({"ShouldInterceptLoadRequest"})
    public void testCalledForImage() throws Throwable {
        final String imagePath = "/" + CommonResources.FAVICON_FILENAME;
        mWebServer.setResponseBase64(imagePath,
                CommonResources.FAVICON_DATA_BASE64, CommonResources.getImagePngHeaders(true));
        final String pageWithImage =
            addPageToTestServer(mWebServer, "/page_with_image.html",
                    CommonResources.getOnImageLoadedHtml(CommonResources.FAVICON_FILENAME));

        int callCount = mShouldInterceptLoadRequestHelper.getCallCount();
        loadUrlSync(pageWithImage);
        mShouldInterceptLoadRequestHelper.waitForCallback(callCount, 2);

        assertEquals(2, mShouldInterceptLoadRequestHelper.getUrls().size());
        assertTrue(mShouldInterceptLoadRequestHelper.getUrls().get(1).endsWith(
                CommonResources.FAVICON_FILENAME));
    }

    @SmallTest
    @Feature({"ShouldInterceptLoadRequest"})
    public void testOnReceivedErrorCallback() throws Throwable {
        final OnReceivedErrorHelper onReceivedErrorHelper = mTestHelperBridge.getOnReceivedErrorHelper();
        mShouldInterceptLoadRequestHelper.setReturnValue(new WebResourceResponse(null, null, null));
        int onReceivedErrorHelperCallCount = onReceivedErrorHelper.getCallCount();
        loadUrlSync("foo://bar");
        onReceivedErrorHelper.waitForCallback(onReceivedErrorHelperCallCount, 1);
    }

    @SmallTest
    @Feature({"ShouldInterceptLoadRequest"})
    public void testNoOnReceivedErrorCallback() throws Throwable {
        final String imagePath = "/" + CommonResources.FAVICON_FILENAME;
        final String imageUrl = mWebServer.setResponseBase64(imagePath,
                CommonResources.FAVICON_DATA_BASE64, CommonResources.getImagePngHeaders(true));
        final String pageWithImage =
                addPageToTestServer(mWebServer, "/page_with_image.html",
                        CommonResources.getOnImageLoadedHtml(CommonResources.FAVICON_FILENAME));
        final OnReceivedErrorHelper onReceivedErrorHelper = mTestHelperBridge.getOnReceivedErrorHelper();
        mShouldInterceptLoadRequestHelper.setReturnValueForUrl(
                imageUrl, new WebResourceResponse(null, null, null));
        int onReceivedErrorHelperCallCount = onReceivedErrorHelper.getCallCount();
        loadUrlSync(pageWithImage);
        assertEquals(onReceivedErrorHelperCallCount, onReceivedErrorHelper.getCallCount());
    }

    @SmallTest
    @Feature({"ShouldInterceptLoadRequest"})
    public void testCalledForIframe() throws Throwable {
        final String aboutPageUrl = addAboutPageToTestServer(mWebServer);
        final String pageWithIframe = addPageToTestServer(mWebServer, "/page_with_iframe.html",
                CommonResources.makeHtmlPageFrom("",
                    "<iframe src=\"" + aboutPageUrl + "\"/>"));

        int callCount = mShouldInterceptLoadRequestHelper.getCallCount();
        // These callbacks can race with favicon.ico callback.
        mShouldInterceptLoadRequestHelper.setUrlToWaitFor(aboutPageUrl);
        loadUrlSync(pageWithIframe);

        mShouldInterceptLoadRequestHelper.waitForCallback(callCount, 1);
        assertEquals(1, mShouldInterceptLoadRequestHelper.getUrls().size());
        assertEquals(aboutPageUrl, mShouldInterceptLoadRequestHelper.getUrls().get(0));
    }

    private void calledForUrlTemplate(final String url) throws Exception {
        int callCount = mShouldInterceptLoadRequestHelper.getCallCount();
        int onPageStartedCallCount = mTestHelperBridge.getOnPageStartedHelper().getCallCount();
        loadUrlAsync(url);
        mShouldInterceptLoadRequestHelper.waitForCallback(callCount);
        assertEquals(url, mShouldInterceptLoadRequestHelper.getUrls().get(0));

        mTestHelperBridge.getOnPageStartedHelper().waitForCallback(onPageStartedCallCount);
        assertEquals(onPageStartedCallCount + 1,
                mTestHelperBridge.getOnPageStartedHelper().getCallCount());
    }

    private void notCalledForUrlTemplate(final String url) throws Exception {
        int callCount = mShouldInterceptLoadRequestHelper.getCallCount();
        loadUrlSync(url);
        // The intercepting must happen before onPageFinished. Since the IPC messages from the
        // renderer should be delivered in order waiting for onPageFinished is sufficient to
        // 'flush' any pending interception messages.
        assertEquals(callCount, mShouldInterceptLoadRequestHelper.getCallCount());
    }

    @SmallTest
    @Feature({"ShouldInterceptLoadRequest"})
    public void testCalledForUnsupportedSchemes() throws Throwable {
        calledForUrlTemplate("foobar://resource/1");
    }

    @SmallTest
    @Feature({"ShouldInterceptLoadRequest"})
    public void testCalledForNonexistentFiles() throws Throwable {
        calledForUrlTemplate("file:///somewhere/something");
    }

    @SmallTest
    @Feature({"ShouldInterceptLoadRequest"})
    public void testCalledForExistingFiles() throws Throwable {
        final String tmpDir = getInstrumentation().getTargetContext().getCacheDir().getPath();
        final String fileName = tmpDir + "/testfile.html";
        final String title = "existing file title";
        TestFileUtil.deleteFile(fileName);  // Remove leftover file if any.
        TestFileUtil.createNewHtmlFile(fileName, title, "");
        final String existingFileUrl = "file://" + fileName;

        int callCount = mShouldInterceptLoadRequestHelper.getCallCount();
        int onPageFinishedCallCount = mTestHelperBridge.getOnPageFinishedHelper().getCallCount();
        loadUrlAsync(existingFileUrl);
        mShouldInterceptLoadRequestHelper.waitForCallback(callCount);
        assertEquals(existingFileUrl, mShouldInterceptLoadRequestHelper.getUrls().get(0));

        mTestHelperBridge.getOnPageFinishedHelper().waitForCallback(onPageFinishedCallCount);
        assertEquals(title, getTitleOnUiThread());
        assertEquals(onPageFinishedCallCount + 1,
                mTestHelperBridge.getOnPageFinishedHelper().getCallCount());
    }

    @SmallTest
    @Feature({"ShouldInterceptLoadRequest"})
    public void testNotCalledForExistingResource() throws Throwable {
        notCalledForUrlTemplate("file:///android_res/raw/resource_file.html");
    }

    @SmallTest
    @Feature({"ShouldInterceptLoadRequest"})
    public void testCalledForNonexistentResource() throws Throwable {
        calledForUrlTemplate("file:///android_res/raw/no_file.html");
    }

    @SmallTest
    @Feature({"ShouldInterceptLoadRequest"})
    public void testNotCalledForExistingAsset() throws Throwable {
        notCalledForUrlTemplate("file:///android_asset/www/index.html");
    }

    @SmallTest
    @Feature({"ShouldInterceptLoadRequest"})
    public void testCalledForNonexistentAsset() throws Throwable {
        calledForUrlTemplate("file:///android_res/raw/no_file.html");
    }

    @SmallTest
    @Feature({"ShouldInterceptLoadRequest"})
    public void testNotCalledForExistingContentUrl() throws Throwable {
        final String contentResourceName = "target";
        final String existingContentUrl = TestContentProvider.createContentUrl(contentResourceName);
        TestContentProvider.resetResourceRequestCount(
                getInstrumentation().getTargetContext(), contentResourceName);

        notCalledForUrlTemplate(existingContentUrl);

        int contentRequestCount = TestContentProvider.getResourceRequestCount(
                getInstrumentation().getTargetContext(), contentResourceName);
        assertEquals(1, contentRequestCount);
    }

    @SmallTest
    @Feature({"ShouldInterceptLoadRequest"})
    public void testCalledForNonexistentContentUrl() throws Throwable {
        calledForUrlTemplate("content://org.xwalk.core.test.NoSuchProvider/foo");
    }

    @SmallTest
    @Feature({"ShouldInterceptLoadRequest"})
    public void testOnPageStartedOnlyOnMainFrame() throws Throwable {
        final String aboutPageUrl = addAboutPageToTestServer(mWebServer);
        final String pageWithIframe = addPageToTestServer(mWebServer, "/page_with_iframe.html",
                CommonResources.makeHtmlPageFrom("",
                    "<iframe src=\"" + aboutPageUrl + "\"/>"));
        int onPageStartedCallCount = mTestHelperBridge.getOnPageStartedHelper().getCallCount();

        loadUrlSync(pageWithIframe);

        mTestHelperBridge.getOnPageStartedHelper().waitForCallback(onPageStartedCallCount);
        assertEquals(onPageStartedCallCount + 1,
                mTestHelperBridge.getOnPageStartedHelper().getCallCount());
    }
}
