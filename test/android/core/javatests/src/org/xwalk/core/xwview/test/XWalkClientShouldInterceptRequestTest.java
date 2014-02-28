// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.graphics.Bitmap;
import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;
import android.util.Pair;
import android.webkit.WebResourceResponse;
import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.Feature;
import org.chromium.base.test.util.TestFileUtil;
import org.chromium.content.browser.test.util.CallbackHelper;
import org.chromium.content.browser.test.util.TestCallbackHelperContainer.OnReceivedErrorHelper;
import org.chromium.net.test.util.TestWebServer;

import org.xwalk.core.xwview.test.util.CommonResources;
import org.xwalk.core.XWalkClient;
import org.xwalk.core.XWalkContent;
import org.xwalk.core.XWalkView;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CountDownLatch;

/**
 * Test case for XWalkClient.shouldInterceptRequest callback
 *
 * Note the major part of this file is migrated from android_webview/.
 */
public class XWalkClientShouldInterceptRequestTest extends XWalkViewTestBase {

    /**
     * Customized XWalkClient implementation for shouldInterceptRequest
     */
    private class TestXWalkClient extends XWalkClient {

        public class ShouldInterceptRequestHelper extends CallbackHelper {
            private List<String> mShouldInterceptRequestUrls = new ArrayList<String>();
            private ConcurrentHashMap<String, WebResourceResponse> mReturnValuesByUrls
                = new ConcurrentHashMap<String, WebResourceResponse>();
            // This is read from the IO thread, so needs to be marked volatile.
            private volatile WebResourceResponse mResourceResponseReturnValue = null;
            private String mUrlToWaitFor;

            void setReturnValue(WebResourceResponse value) {
                mResourceResponseReturnValue = value;
            }

            void setReturnValueForUrl(String url, WebResourceResponse value) {
                mReturnValuesByUrls.put(url, value);
            }

            public void setUrlToWaitFor(String url) {
                mUrlToWaitFor = url;
            }

            public List<String> getUrls() {
                assert getCallCount() > 0;
                return mShouldInterceptRequestUrls;
            }

            public WebResourceResponse getReturnValue(String url) {
                WebResourceResponse value = mReturnValuesByUrls.get(url);
                if (value != null) return value;
                return mResourceResponseReturnValue;
            }

            public void notifyCalled(String url) {
                if (mUrlToWaitFor == null || mUrlToWaitFor.equals(url)) {
                    mShouldInterceptRequestUrls.add(url);
                    notifyCalled();
                }
            }
        }

        public class OnLoadResourceHelper extends CallbackHelper {
            private String mUrl;

            public String getUrl() {
                assert getCallCount() > 0;
                return mUrl;
            }

            public void notifyCalled(String url) {
                mUrl = url;
                notifyCalled();
            }
        }

        @Override
        public void onPageStarted(XWalkView view, String url, Bitmap favicon) {
            mTestContentsClient.onPageStarted(url);
        }

        @Override
        public void onPageFinished(XWalkView view, String url) {
            mTestContentsClient.didFinishLoad(url);
        }

        @Override
        public void onReceivedError(XWalkView view, int errorCode,
                String description, String failingUrl) {
            mTestContentsClient.onReceivedError(errorCode, description, failingUrl);
        }

        @Override
        public WebResourceResponse shouldInterceptRequest(XWalkView view, String url) {
            WebResourceResponse response = mShouldInterceptRequestHelper.getReturnValue(url);
            mShouldInterceptRequestHelper.notifyCalled(url);
            return response;
        }

        @Override
        public void onLoadResource(XWalkView view, String url) {
            super.onLoadResource(view, url);
            mOnLoadResourceHelper.notifyCalled(url);
        }

        private ShouldInterceptRequestHelper mShouldInterceptRequestHelper;
        private OnLoadResourceHelper mOnLoadResourceHelper;

        public TestXWalkClient() {
            mShouldInterceptRequestHelper = new ShouldInterceptRequestHelper();
            mOnLoadResourceHelper = new OnLoadResourceHelper();
        }

        public ShouldInterceptRequestHelper getShouldInterceptRequestHelper() {
            return mShouldInterceptRequestHelper;
        }

        public OnLoadResourceHelper getOnLoadResourceHelper() {
            return mOnLoadResourceHelper;
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
    private TestXWalkClient mTestXWalkClient;
    private TestXWalkClient.ShouldInterceptRequestHelper mShouldInterceptRequestHelper;
    private TestXWalkClient.OnLoadResourceHelper mOnLoadResourceHelper;

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mTestXWalkClient = new TestXWalkClient();
                getXWalkView().setXWalkClient(mTestXWalkClient);
                mShouldInterceptRequestHelper = mTestXWalkClient.getShouldInterceptRequestHelper();
                mOnLoadResourceHelper = mTestXWalkClient.getOnLoadResourceHelper();
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
    @Feature({"XWalkClientShouldInterceptRequest"})
    public void testCalledWithCorrectUrl() throws Throwable {
        final String aboutPageUrl = addAboutPageToTestServer(mWebServer);

        int callCount = mShouldInterceptRequestHelper.getCallCount();
        int onPageFinishedCallCount = mTestContentsClient.getOnPageFinishedHelper().getCallCount();

        loadUrlAsync(aboutPageUrl);

        mShouldInterceptRequestHelper.waitForCallback(callCount);
        assertEquals(1, mShouldInterceptRequestHelper.getUrls().size());
        assertEquals(aboutPageUrl,
                mShouldInterceptRequestHelper.getUrls().get(0));

        mTestContentsClient.getOnPageFinishedHelper().waitForCallback(onPageFinishedCallCount);
        assertEquals(CommonResources.ABOUT_TITLE, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"XWalkClientShouldInterceptRequest"})
    public void testOnLoadResourceCalledWithCorrectUrl() throws Throwable {
        final String aboutPageUrl = addAboutPageToTestServer(mWebServer);
        int callCount = mOnLoadResourceHelper.getCallCount();

        loadUrlAsync(aboutPageUrl);

        mOnLoadResourceHelper.waitForCallback(callCount);
        assertEquals(aboutPageUrl, mOnLoadResourceHelper.getUrl());
    }

    @SmallTest
    @Feature({"XWalkClientShouldInterceptRequest"})
    public void testDoesNotCrashOnInvalidData() throws Throwable {
        final String aboutPageUrl = addAboutPageToTestServer(mWebServer);

        mShouldInterceptRequestHelper.setReturnValue(
                new WebResourceResponse("text/html", "UTF-8", null));
        int callCount = mShouldInterceptRequestHelper.getCallCount();
        loadUrlAsync(aboutPageUrl);
        mShouldInterceptRequestHelper.waitForCallback(callCount);

        mShouldInterceptRequestHelper.setReturnValue(
                new WebResourceResponse(null, null, new ByteArrayInputStream(new byte[0])));
        callCount = mShouldInterceptRequestHelper.getCallCount();
        loadUrlAsync(aboutPageUrl);
        mShouldInterceptRequestHelper.waitForCallback(callCount);

        mShouldInterceptRequestHelper.setReturnValue(
                new WebResourceResponse(null, null, null));
        callCount = mShouldInterceptRequestHelper.getCallCount();
        loadUrlAsync(aboutPageUrl);
        mShouldInterceptRequestHelper.waitForCallback(callCount);
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
    @Feature({"XWalkClientShouldInterceptRequest"})
    public void testDoesNotCrashOnEmptyStream() throws Throwable {
        final String aboutPageUrl = addAboutPageToTestServer(mWebServer);

        mShouldInterceptRequestHelper.setReturnValue(
                new WebResourceResponse("text/html", "UTF-8", new EmptyInputStream()));
        int shouldInterceptRequestCallCount = mShouldInterceptRequestHelper.getCallCount();
        int onPageFinishedCallCount = mTestContentsClient.getOnPageFinishedHelper().getCallCount();

        loadUrlAsync(aboutPageUrl);

        mShouldInterceptRequestHelper.waitForCallback(shouldInterceptRequestCallCount);
        mTestContentsClient.getOnPageFinishedHelper().waitForCallback(onPageFinishedCallCount);
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

    //@SmallTest
    //@Feature({"XWalkClientShouldInterceptRequest"})
    // TODO(gaochun): Enable it once the issue XWALK-1022 gets resolved.
    @DisabledTest
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

        mShouldInterceptRequestHelper.setReturnValue(
                new WebResourceResponse("text/html", "UTF-8", null));
        assertEquals("404", executeJavaScriptAndWaitForResult(syncGetJs));

        mShouldInterceptRequestHelper.setReturnValue(
                new WebResourceResponse("text/html", "UTF-8", new EmptyInputStream()));
        assertEquals("200", executeJavaScriptAndWaitForResult(syncGetJs));
    }


    private String makePageWithTitle(String title) {
        return CommonResources.makeHtmlPageFrom("<title>" + title + "</title>",
                "<div> The title is: " + title + " </div>");
    }

    @SmallTest
    @Feature({"XWalkClientShouldInterceptRequest"})
    public void testCanInterceptMainFrame() throws Throwable {
        final String expectedTitle = "testShouldInterceptRequestCanInterceptMainFrame";
        final String expectedPage = makePageWithTitle(expectedTitle);

        mShouldInterceptRequestHelper.setReturnValue(
                stringToWebResourceResponse(expectedPage));

        final String aboutPageUrl = addAboutPageToTestServer(mWebServer);

        loadUrlSync(aboutPageUrl);

        assertEquals(expectedTitle, getTitleOnUiThread());
        assertEquals(0, mWebServer.getRequestCount("/" + CommonResources.ABOUT_FILENAME));
    }

    @SmallTest
    @Feature({"XWalkClientShouldInterceptRequest"})
    public void testDoesNotChangeReportedUrl() throws Throwable {
        mShouldInterceptRequestHelper.setReturnValue(
                stringToWebResourceResponse(makePageWithTitle("some title")));

        final String aboutPageUrl = addAboutPageToTestServer(mWebServer);

        loadUrlSync(aboutPageUrl);

        assertEquals(aboutPageUrl, mTestContentsClient.getOnPageFinishedHelper().getUrl());
        assertEquals(aboutPageUrl, mTestContentsClient.getOnPageStartedHelper().getUrl());
    }

    @SmallTest
    @Feature({"XWalkClientShouldInterceptRequest"})
    public void testNullInputStreamCausesErrorForMainFrame() throws Throwable {
        final OnReceivedErrorHelper onReceivedErrorHelper = mTestContentsClient.getOnReceivedErrorHelper();
        mShouldInterceptRequestHelper.setReturnValue(
                new WebResourceResponse("text/html", "UTF-8", null));

        final String aboutPageUrl = addAboutPageToTestServer(mWebServer);
        final int callCount = onReceivedErrorHelper.getCallCount();
        loadUrlAsync(aboutPageUrl);
        onReceivedErrorHelper.waitForCallback(callCount);
        assertEquals(0, mWebServer.getRequestCount("/" + CommonResources.ABOUT_FILENAME));
    }

    @SmallTest
    @Feature({"XWalkClientShouldInterceptRequest"})
    public void testCalledForImage() throws Throwable {
        final String imagePath = "/" + CommonResources.FAVICON_FILENAME;
        mWebServer.setResponseBase64(imagePath,
                CommonResources.FAVICON_DATA_BASE64, CommonResources.getImagePngHeaders(true));
        final String pageWithImage =
            addPageToTestServer(mWebServer, "/page_with_image.html",
                    CommonResources.getOnImageLoadedHtml(CommonResources.FAVICON_FILENAME));

        int callCount = mShouldInterceptRequestHelper.getCallCount();
        loadUrlSync(pageWithImage);
        mShouldInterceptRequestHelper.waitForCallback(callCount, 2);

        assertEquals(2, mShouldInterceptRequestHelper.getUrls().size());
        assertTrue(mShouldInterceptRequestHelper.getUrls().get(1).endsWith(
                CommonResources.FAVICON_FILENAME));
    }

    @SmallTest
    @Feature({"XWalkClientShouldInterceptRequest"})
    public void testOnReceivedErrorCallback() throws Throwable {
        final OnReceivedErrorHelper onReceivedErrorHelper = mTestContentsClient.getOnReceivedErrorHelper();
        mShouldInterceptRequestHelper.setReturnValue(new WebResourceResponse(null, null, null));
        int onReceivedErrorHelperCallCount = onReceivedErrorHelper.getCallCount();
        loadUrlSync("foo://bar");
        onReceivedErrorHelper.waitForCallback(onReceivedErrorHelperCallCount, 1);
    }

    @SmallTest
    @Feature({"XWalkClientShouldInterceptRequest"})
    public void testNoOnReceivedErrorCallback() throws Throwable {
        final String imagePath = "/" + CommonResources.FAVICON_FILENAME;
        final String imageUrl = mWebServer.setResponseBase64(imagePath,
                CommonResources.FAVICON_DATA_BASE64, CommonResources.getImagePngHeaders(true));
        final String pageWithImage =
                addPageToTestServer(mWebServer, "/page_with_image.html",
                        CommonResources.getOnImageLoadedHtml(CommonResources.FAVICON_FILENAME));
        final OnReceivedErrorHelper onReceivedErrorHelper = mTestContentsClient.getOnReceivedErrorHelper();
        mShouldInterceptRequestHelper.setReturnValueForUrl(
                imageUrl, new WebResourceResponse(null, null, null));
        int onReceivedErrorHelperCallCount = onReceivedErrorHelper.getCallCount();
        loadUrlSync(pageWithImage);
        assertEquals(onReceivedErrorHelperCallCount, onReceivedErrorHelper.getCallCount());
    }

    @SmallTest
    @Feature({"XWalkClientShouldInterceptRequest"})
    public void testCalledForIframe() throws Throwable {
        final String aboutPageUrl = addAboutPageToTestServer(mWebServer);
        final String pageWithIframe = addPageToTestServer(mWebServer, "/page_with_iframe.html",
                CommonResources.makeHtmlPageFrom("",
                    "<iframe src=\"" + aboutPageUrl + "\"/>"));

        int callCount = mShouldInterceptRequestHelper.getCallCount();
        // These callbacks can race with favicon.ico callback.
        mShouldInterceptRequestHelper.setUrlToWaitFor(aboutPageUrl);
        loadUrlSync(pageWithIframe);

        mShouldInterceptRequestHelper.waitForCallback(callCount, 1);
        assertEquals(1, mShouldInterceptRequestHelper.getUrls().size());
        assertEquals(aboutPageUrl, mShouldInterceptRequestHelper.getUrls().get(0));
    }

    private void calledForUrlTemplate(final String url) throws Exception {
        int callCount = mShouldInterceptRequestHelper.getCallCount();
        int onPageStartedCallCount = mTestContentsClient.getOnPageStartedHelper().getCallCount();
        loadUrlAsync(url);
        mShouldInterceptRequestHelper.waitForCallback(callCount);
        assertEquals(url, mShouldInterceptRequestHelper.getUrls().get(0));

        mTestContentsClient.getOnPageStartedHelper().waitForCallback(onPageStartedCallCount);
        assertEquals(onPageStartedCallCount + 1,
                mTestContentsClient.getOnPageStartedHelper().getCallCount());
    }

    private void notCalledForUrlTemplate(final String url) throws Exception {
        int callCount = mShouldInterceptRequestHelper.getCallCount();
        loadUrlSync(url);
        // The intercepting must happen before onPageFinished. Since the IPC messages from the
        // renderer should be delivered in order waiting for onPageFinished is sufficient to
        // 'flush' any pending interception messages.
        assertEquals(callCount, mShouldInterceptRequestHelper.getCallCount());
    }

    @SmallTest
    @Feature({"XWalkClientShouldInterceptRequest"})
    public void testCalledForUnsupportedSchemes() throws Throwable {
        calledForUrlTemplate("foobar://resource/1");
    }

    @SmallTest
    @Feature({"XWalkClientShouldInterceptRequest"})
    public void testCalledForNonexistentFiles() throws Throwable {
        calledForUrlTemplate("file:///somewhere/something");
    }

    @SmallTest
    @Feature({"XWalkClientShouldInterceptRequest"})
    public void testCalledForExistingFiles() throws Throwable {
        final String tmpDir = getInstrumentation().getTargetContext().getCacheDir().getPath();
        final String fileName = tmpDir + "/testfile.html";
        final String title = "existing file title";
        TestFileUtil.deleteFile(fileName);  // Remove leftover file if any.
        TestFileUtil.createNewHtmlFile(fileName, title, "");
        final String existingFileUrl = "file://" + fileName;

        int callCount = mShouldInterceptRequestHelper.getCallCount();
        int onPageFinishedCallCount = mTestContentsClient.getOnPageFinishedHelper().getCallCount();
        loadUrlAsync(existingFileUrl);
        mShouldInterceptRequestHelper.waitForCallback(callCount);
        assertEquals(existingFileUrl, mShouldInterceptRequestHelper.getUrls().get(0));

        mTestContentsClient.getOnPageFinishedHelper().waitForCallback(onPageFinishedCallCount);
        assertEquals(title, getTitleOnUiThread());
        assertEquals(onPageFinishedCallCount + 1,
                mTestContentsClient.getOnPageFinishedHelper().getCallCount());
    }

    @SmallTest
    @Feature({"XWalkClientShouldInterceptRequest"})
    public void testNotCalledForExistingResource() throws Throwable {
        notCalledForUrlTemplate("file:///android_res/raw/resource_file.html");
    }

    @SmallTest
    @Feature({"XWalkClientShouldInterceptRequest"})
    public void testCalledForNonexistentResource() throws Throwable {
        calledForUrlTemplate("file:///android_res/raw/no_file.html");
    }

    @SmallTest
    @Feature({"XWalkClientShouldInterceptRequest"})
    public void testNotCalledForExistingAsset() throws Throwable {
        notCalledForUrlTemplate("file:///android_asset/www/index.html");
    }

    @SmallTest
    @Feature({"XWalkClientShouldInterceptRequest"})
    public void testCalledForNonexistentAsset() throws Throwable {
        calledForUrlTemplate("file:///android_res/raw/no_file.html");
    }

    @SmallTest
    @Feature({"XWalkClientShouldInterceptRequest"})
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
    @Feature({"XWalkClientShouldInterceptRequest"})
    public void testCalledForNonexistentContentUrl() throws Throwable {
        calledForUrlTemplate("content://org.xwalk.core.test.NoSuchProvider/foo");
    }
}
