// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.net.http.SslCertificate;
import android.net.http.SslError;
import android.net.Uri;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.view.KeyEvent;
import android.webkit.ValueCallback;
import android.webkit.WebResourceResponse;

import java.io.File;
import java.io.InputStream;
import java.io.IOException;
import java.util.concurrent.atomic.AtomicReference;
import java.util.concurrent.Callable;
import java.util.concurrent.FutureTask;
import java.util.concurrent.TimeUnit;

import junit.framework.Assert;

import org.chromium.base.test.util.TestFileUtil;
import org.chromium.base.test.util.UrlUtils;
import org.chromium.content_public.browser.LoadUrlParams;
import org.chromium.content.browser.test.util.CallbackHelper;
import org.chromium.content.browser.test.util.Criteria;
import org.chromium.content.browser.test.util.CriteriaHelper;
import org.chromium.net.test.util.TestWebServer;
import org.chromium.ui.gfx.DeviceDisplayInfo;

import org.xwalk.core.ClientCertRequest;
import org.xwalk.core.XWalkDownloadListener;
import org.xwalk.core.XWalkHttpAuthHandler;
import org.xwalk.core.XWalkJavascriptResult;
import org.xwalk.core.XWalkNavigationHistory;
import org.xwalk.core.XWalkNavigationItem;
import org.xwalk.core.XWalkResourceClient;
import org.xwalk.core.XWalkSettings;
import org.xwalk.core.XWalkSettings.LayoutAlgorithm;
import org.xwalk.core.XWalkUIClient;
import org.xwalk.core.XWalkView;
import org.xwalk.core.XWalkWebResourceRequest;
import org.xwalk.core.XWalkWebResourceResponse;

import org.xwalk.core.xwview.test.util.ImagePageGenerator;

public class XWalkViewTestBase
       extends ActivityInstrumentationTestCase2<XWalkViewTestRunnerActivity> {
    protected final static int WAIT_TIMEOUT_SECONDS = 15;
    protected final static long WAIT_TIMEOUT_MS = 2000;
    private final static int CHECK_INTERVAL = 100;
    private final static String TAG = "XWalkViewTestBase";
    private XWalkView mXWalkView;
    private boolean mAllowSslError = true;
    final TestHelperBridge mTestHelperBridge = new TestHelperBridge();
    private static final boolean ENABLED = true;
    private static final boolean DISABLED = false;

    class TestXWalkUIClientBase extends XWalkUIClient {
        TestHelperBridge mInnerContentsClient;
        public TestXWalkUIClientBase(TestHelperBridge client) {
            super(getXWalkView());
            mInnerContentsClient = client;
        }

        @Override
        public void onPageLoadStarted(XWalkView view, String url) {
            mInnerContentsClient.onPageStarted(url);
        }

        @Override
        public void onPageLoadStopped(XWalkView view, String url, LoadStatus status) {
            mInnerContentsClient.onPageFinished(url, status);
        }

        @Override
        public void onReceivedTitle(XWalkView view, String title) {
            mInnerContentsClient.onTitleChanged(title);
        }

        @Override
        public void onJavascriptCloseWindow(XWalkView view) {
            mInnerContentsClient.onJavascriptCloseWindow();
        }

        @Override
        public void onScaleChanged(XWalkView view, float oldScale, float newScale) {
            mInnerContentsClient.onScaleChanged(oldScale, newScale);
        }

        @Override
        public void onRequestFocus(XWalkView view) {
            mInnerContentsClient.onRequestFocus();
        }

        @Override
        public boolean onJavascriptModalDialog(XWalkView view,
                XWalkUIClient.JavascriptMessageType type, String url, String message,
                        String defaultValue, XWalkJavascriptResult result) {
            /**
             * NOTE: Since onJavascriptModalDialog, onJsAlert, onJsConfirm,
             *       and onJsPrompt API are overriden in the same subclass,
             *       call onJsAlert, onJsConfirm and onJsPrompt here to invoke
             *       overriden implementation.
             **/
            switch(type) {
                case JAVASCRIPT_ALERT:
                     onJsAlert(view, url, message, result);
                     break;
                case JAVASCRIPT_CONFIRM:
                     onJsConfirm(view, url, message, result);
                     break;
                case JAVASCRIPT_PROMPT:
                     onJsPrompt(view, url, message, defaultValue, result);
                     break;
                case JAVASCRIPT_BEFOREUNLOAD:
                    // Reuse onJsConfirm to show the dialog.
                    onJsConfirm(view, url, message, result);
                    break;
                default:
                    break;
            }

            return mInnerContentsClient.onJavascriptModalDialog(message);
        }

        @Override
        public boolean onJsAlert(XWalkView view,
                String url, String message, XWalkJavascriptResult result) {
            return mInnerContentsClient.onJsAlert(message);
        }

        @Override
        public boolean onJsConfirm(XWalkView view,
                String url, String message, XWalkJavascriptResult result) {
            return mInnerContentsClient.onJsConfirm(message);
        }

        @Override
        public boolean onJsPrompt(XWalkView view,
                String url, String message, String defaultValue,
                XWalkJavascriptResult result) {
            return mInnerContentsClient.onJsPrompt(message);
        }

        @Override
        public boolean onConsoleMessage(XWalkView view, String message,
                int lineNumber, String sourceId, ConsoleMessageType messageType) {
            return mInnerContentsClient.onConsoleMessage(message,lineNumber,sourceId,
                    messageType);
        }

        @Override
        public void openFileChooser(XWalkView view, ValueCallback<Uri> uploadFile,
                String acceptType, String capture) {
            mInnerContentsClient.openFileChooser(uploadFile);
        }

        @Override
        public void onFullscreenToggled(XWalkView view, boolean enterFullscreen) {
            mInnerContentsClient.onFullscreenToggled(enterFullscreen);
        }

        @Override
        public boolean shouldOverrideKeyEvent(XWalkView view, KeyEvent event) {
            return mInnerContentsClient.overrideOrUnhandledKeyEvent(event);
        }
    }

    class TestXWalkUIClient extends TestXWalkUIClientBase {
        public TestXWalkUIClient() {
            super(mTestHelperBridge);
        }
    }

    class TestXWalkResourceClientBase extends XWalkResourceClient {
        TestHelperBridge mInnerContentsClient;
        public TestXWalkResourceClientBase(TestHelperBridge client) {
            super(mXWalkView);
            mInnerContentsClient = client;
        }

        @Override
        public void onLoadStarted(XWalkView view, String url) {
            mInnerContentsClient.onLoadStarted(url);
        }

        @Override
        public void onReceivedLoadError(XWalkView view, int errorCode, String description, String failingUrl) {
            mInnerContentsClient.onReceivedLoadError(errorCode, description, failingUrl);
        }

        @Override
        public XWalkWebResourceResponse shouldInterceptLoadRequest(XWalkView view,
                XWalkWebResourceRequest request) {
            return mInnerContentsClient.shouldInterceptLoadRequest(request.getUrl().toString());
        }

        @Override
        public void onProgressChanged(XWalkView view, int progressInPercent) {
            mTestHelperBridge.onProgressChanged(progressInPercent);
        }

        @Override
        public boolean shouldOverrideUrlLoading(XWalkView view, String url) {
            return mTestHelperBridge.shouldOverrideUrlLoading(url);
        }

        @Override
        public void onLoadFinished(XWalkView view, String url) {
            mInnerContentsClient.onLoadFinished(url);
        }

        @Override
        public void onDocumentLoadedInFrame(XWalkView view, long frameId) {
            mInnerContentsClient.onDocumentLoadedInFrame(frameId);
        }

        @Override
        public void onReceivedClientCertRequest(XWalkView view, ClientCertRequest handler)  {
            mTestHelperBridge.onReceivedClientCertRequest(view, handler);
        }

        @Override
        public void onReceivedResponseHeaders(XWalkView view,
                XWalkWebResourceRequest request,
                XWalkWebResourceResponse response) {
            mTestHelperBridge.onReceivedResponseHeaders(view, request, response);
        }

        @Override
        public void onReceivedHttpAuthRequest(XWalkView view,
                XWalkHttpAuthHandler handler, String host, String realm) {
            mInnerContentsClient.onReceivedHttpAuthRequest(host);
        }

        @Override
        public void onReceivedSslError(XWalkView view, ValueCallback<Boolean> callback,
                SslError error) {
            callback.onReceiveValue(mAllowSslError);
            mTestHelperBridge.onReceivedSslError(callback, error);
        }
    }

    class TestXWalkResourceClient extends TestXWalkResourceClientBase {
        public TestXWalkResourceClient() {
            super(mTestHelperBridge);
        }
    }

    class TestXWalkDownloadListener extends XWalkDownloadListener {
        public TestXWalkDownloadListener(Context context) {
            super(context);
        }

        @Override
        public void onDownloadStart(String url, String userAgent,
                String contentDisposition, String mimetype, long contentLength) {
            mTestHelperBridge.onDownloadStart(url, userAgent, contentDisposition,
                    mimetype, contentLength);
        }
    }

    void setDownloadListener() {
        final Context context = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                TestXWalkDownloadListener listener = new TestXWalkDownloadListener(context);
                getXWalkView().setDownloadListener(listener);
            }
        });
    }

    void setUIClient(final XWalkUIClient client) {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getXWalkView().setUIClient(client);
            }
        });
    }

    void setResourceClient(final XWalkResourceClient client) {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getXWalkView().setResourceClient(client);
            }
        });
    }

    public XWalkViewTestBase() {
        super(XWalkViewTestRunnerActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        // Must call getActivity() here but not in main thread.
        final Activity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView = new XWalkView(activity, activity);
                getActivity().addView(mXWalkView);
                mXWalkView.setUIClient(new TestXWalkUIClient());
                mXWalkView.setResourceClient(new TestXWalkResourceClient());
                // mXWalkView.getXWalkViewContentForTest().installWebContentsObserverForTest(mTestHelperBridge);
            }
        });
    }

    protected void pollOnUiThread(final Callable<Boolean> callable) throws Exception {
        poll(new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return runTestOnUiThreadAndGetResult(callable);
            }
        });
    }

    protected void loadJavaScriptUrl(final String url) throws Exception {
        if (!url.startsWith("javascript:")) {
            Log.w(TAG, "loadJavascriptUrl only accepts javascript: url");
            return;
        }
        loadUrlAsync(url);
    }

    protected void loadUrlSync(final String url) throws Exception {
        CallbackHelper pageFinishedHelper = mTestHelperBridge.getOnPageFinishedHelper();
        int currentCallCount = pageFinishedHelper.getCallCount();
        loadUrlAsync(url);
        pageFinishedHelper.waitForCallback(currentCallCount, 1, WAIT_TIMEOUT_SECONDS,
                TimeUnit.SECONDS);
    }


    protected void loadUrlSyncAndExpectError(final String url) throws Exception {
        CallbackHelper onPageFinishedHelper = mTestHelperBridge.getOnPageFinishedHelper();
        CallbackHelper onReceivedErrorHelper = mTestHelperBridge.getOnReceivedErrorHelper();
        int onErrorCallCount = onReceivedErrorHelper.getCallCount();
        int onFinishedCallCount = onPageFinishedHelper.getCallCount();
        loadUrlAsync(url);
        onReceivedErrorHelper.waitForCallback(onErrorCallCount, 1, WAIT_TIMEOUT_MS,
                TimeUnit.MILLISECONDS);
        onPageFinishedHelper.waitForCallback(onFinishedCallCount, 1, WAIT_TIMEOUT_MS,
                TimeUnit.MILLISECONDS);
    }

    protected void loadUrlAsync(final String url) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.loadUrl(url);
            }
        });
    }

    protected void loadDataSync(final String data, final String mimeType,
            final boolean isBase64Encoded) throws Exception {
        CallbackHelper pageFinishedHelper = mTestHelperBridge.getOnPageFinishedHelper();
        int currentCallCount = pageFinishedHelper.getCallCount();
        loadDataAsync(data, mimeType, isBase64Encoded);
        pageFinishedHelper.waitForCallback(currentCallCount, 1, WAIT_TIMEOUT_SECONDS,
                TimeUnit.SECONDS);
    }

    protected void loadDataAsync(final String data, final String mimeType,
             final boolean isBase64Encoded) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.loadData(data, mimeType, isBase64Encoded ? "base64" : null);
            }
        });
    }

    protected void loadDataWithBaseUrlSync(final String data, final String mimeType,
            final boolean isBase64Encoded, final String baseUrl,
            final String historyUrl) throws Throwable {
        CallbackHelper pageFinishedHelper = mTestHelperBridge.getOnPageFinishedHelper();
        int currentCallCount = pageFinishedHelper.getCallCount();
        loadDataWithBaseUrlAsync(data, mimeType, isBase64Encoded, baseUrl, historyUrl);
        pageFinishedHelper.waitForCallback(currentCallCount, 1, WAIT_TIMEOUT_SECONDS,
                TimeUnit.SECONDS);
    }

    protected void loadDataWithBaseUrlAsync(final String data, final String mimeType,
            final boolean isBase64Encoded, final String baseUrl, final String historyUrl) throws Throwable {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.loadDataWithBaseURL(
                        baseUrl, data, mimeType, isBase64Encoded ? "base64" : null, historyUrl);
            }
        });
    }

    protected void loadUrlSyncByContent(final XWalkView xWalkContent,
            final TestHelperBridge contentsClient,
            final String url) throws Exception {
        CallbackHelper pageFinishedHelper = contentsClient.getOnPageFinishedHelper();
        int currentCallCount = pageFinishedHelper.getCallCount();
        loadUrlAsyncByContent(xWalkContent, url);
        pageFinishedHelper.waitForCallback(currentCallCount, 1, WAIT_TIMEOUT_SECONDS,
                TimeUnit.SECONDS);
    }

    protected void loadUrlSyncByContentAndExpectError(final XWalkView xWalkContent,
            final TestHelperBridge contentsClient,
            final String url) throws Exception {
        CallbackHelper onPageFinishedHelper = contentsClient.getOnPageFinishedHelper();
        CallbackHelper onReceivedErrorHelper = contentsClient.getOnReceivedErrorHelper();
        int onErrorCallCount = onReceivedErrorHelper.getCallCount();
        int onFinishedCallCount = onPageFinishedHelper.getCallCount();
        loadUrlAsyncByContent(xWalkContent, url);
        onReceivedErrorHelper.waitForCallback(onErrorCallCount, 1, WAIT_TIMEOUT_MS,
                TimeUnit.MILLISECONDS);
        onPageFinishedHelper.waitForCallback(onFinishedCallCount, 1, WAIT_TIMEOUT_MS,
                TimeUnit.MILLISECONDS);
    }

    protected void loadUrlAsyncByContent(final XWalkView xWalkContent,
            final String url) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                xWalkContent.loadUrl(url);
            }
        });
    }

    protected String getTitleOnUiThread() throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<String>() {
            @Override
            public String call() throws Exception {
                return mXWalkView.getTitle();
            }
        });
    }

    protected Bitmap getFaviconOnUiThread() throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<Bitmap>() {
            @Override
            public Bitmap call() throws Exception {
                return mXWalkView.getFavicon();
            }
        });
    }

    protected <R> R runTestOnUiThreadAndGetResult(Callable<R> callable)
            throws Exception {
        FutureTask<R> task = new FutureTask<R>(callable);
        getInstrumentation().waitForIdleSync();
        getInstrumentation().runOnMainSync(task);
        return task.get();
    }

    protected String getFileContent(String fileName) {
        try {
            Context context = getInstrumentation().getContext();
            InputStream inputStream = context.getAssets().open(fileName);
            int size = inputStream.available();
            byte buffer[] = new byte[size];
            inputStream.read(buffer);
            inputStream.close();

            String fileContent = new String(buffer);
            return fileContent;
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    protected String getTitleOnUiThreadByContent(final XWalkView xWalkContent) throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<String>() {
            @Override
            public String call() throws Exception {
                String title = xWalkContent.getTitle();
                return title;
            }
        });
    }

    protected XWalkView createXWalkViewContainerOnMainSync(
            final Context context,
            final XWalkUIClient uiClient,
            final XWalkResourceClient resourceClient) throws Exception {
        final AtomicReference<XWalkView> xWalkViewContainer =
                new AtomicReference<XWalkView>();
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                xWalkViewContainer.set(new XWalkView(context, getActivity()));
                getActivity().addView(xWalkViewContainer.get());
                xWalkViewContainer.get().setUIClient(uiClient);
                xWalkViewContainer.get().setResourceClient(resourceClient);
            }
        });

        return xWalkViewContainer.get();
    }

    protected void loadAssetFile(String fileName) throws Exception {
        String fileContent = getFileContent(fileName);
        loadDataSync(fileContent, "text/html", false);
    }

    public void loadAssetFileAndWaitForTitle(String fileName) throws Exception {
        CallbackHelper getTitleHelper = mTestHelperBridge.getOnTitleUpdatedHelper();
        int currentCallCount = getTitleHelper.getCallCount();
        String fileContent = getFileContent(fileName);
        loadDataAsync(fileContent, "text/html", false);
        getTitleHelper.waitForCallback(currentCallCount, 1, WAIT_TIMEOUT_SECONDS,
                TimeUnit.SECONDS);
    }

    protected XWalkView getXWalkView() {
        return mXWalkView;
    }

    protected void runTestWaitPageFinished(Runnable runnable) throws Exception{
        CallbackHelper pageFinishedHelper = mTestHelperBridge.getOnPageFinishedHelper();
        int currentCallCount = pageFinishedHelper.getCallCount();
        runnable.run();
        pageFinishedHelper.waitForCallback(currentCallCount, 1, WAIT_TIMEOUT_SECONDS,
                TimeUnit.SECONDS);
    }

    protected void reloadSync(final int mode) throws Exception {
        runTestWaitPageFinished(new Runnable(){
            @Override
            public void run() {
                getInstrumentation().runOnMainSync(new Runnable() {
                    @Override
                    public void run() {
                        mXWalkView.reload(mode);
                    }
                });
            }
        });
    }

    protected void goBackSync() throws Throwable {
        runTestWaitPageFinished(new Runnable(){
            @Override
            public void run() {
                try {
                    goBackAsync();
                } catch (Throwable t) {
                    t.printStackTrace();
                }
            }
        });
    }

    protected void goForwardSync() throws Throwable {
        runTestWaitPageFinished(new Runnable(){
            @Override
            public void run() {
                try {
                    goForwardAsync();
                } catch (Throwable t) {
                    t.printStackTrace();
                }
            }
        });
    }

    protected void goBackAsync() throws Throwable {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.getNavigationHistory().navigate(
                    XWalkNavigationHistory.Direction.BACKWARD, 1);
            }
        });
    }

    protected void goForwardAsync() throws Throwable {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.getNavigationHistory().navigate(
                    XWalkNavigationHistory.Direction.FORWARD, 1);
            }
        });
    }

    protected void clearHistoryOnUiThread() throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.getNavigationHistory().clear();
            }
        });
    }

    protected boolean canGoBackOnUiThread() throws Throwable {
        return runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() {
                return mXWalkView.getNavigationHistory().canGoBack();
            }
        });
    }

    protected boolean canGoForwardOnUiThread() throws Throwable {
        return runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() {
                return mXWalkView.getNavigationHistory().canGoForward();
            }
        });
    }

    protected int historySizeOnUiThread() throws Throwable {
        return runTestOnUiThreadAndGetResult(new Callable<Integer>() {
            @Override
            public Integer call() {
                return mXWalkView.getNavigationHistory().size();
            }
        });
    }

    protected boolean hasItemAtOnUiThread(final int index) throws Throwable {
        return runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() {
                return mXWalkView.getNavigationHistory().hasItemAt(index);
            }
        });
    }

    protected XWalkNavigationItem getItemAtOnUiThread(final int index) throws Throwable {
        return runTestOnUiThreadAndGetResult(new Callable<XWalkNavigationItem>() {
            @Override
            public XWalkNavigationItem call() {
                return mXWalkView.getNavigationHistory().getItemAt(index);
            }
        });
    }

    protected int getContentHeightOnUiThread() throws Throwable {
        return runTestOnUiThreadAndGetResult(new Callable<Integer>() {
            @Override
            public Integer call() {
                return mXWalkView.getContentHeight();
            }
        });
    }

    protected XWalkNavigationItem getCurrentItemOnUiThread() throws Throwable {
        return runTestOnUiThreadAndGetResult(new Callable<XWalkNavigationItem>() {
            @Override
            public XWalkNavigationItem call() {
                return mXWalkView.getNavigationHistory().getCurrentItem();
            }
        });
    }

    protected String executeJavaScriptAndWaitForResult(final String code) throws Exception {

        final TestHelperBridge.OnEvaluateJavaScriptResultHelper helper =
                mTestHelperBridge.getOnEvaluateJavaScriptResultHelper();
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                helper.evaluateJavascript(mXWalkView, code);
            }
        });
        helper.waitUntilHasValue();
        Assert.assertTrue("Failed to retrieve JavaScript evaluation results.", helper.hasValue());
        return helper.getJsonResultAndClear();
    }

    /**
     * Wrapper around CriteriaHelper.pollInstrumentationThread. This uses XWalkViewTestBase-specifc
     * timeouts and treats timeouts and exceptions as test failures automatically.
     */
    protected static void pollInstrumentationThread(final Callable<Boolean> callable)
            throws Exception {
        CriteriaHelper.pollInstrumentationThread(new Criteria() {
            @Override
            public boolean isSatisfied() {
                try {
                    return callable.call();
                } catch (Throwable e) {
                    Log.e(TAG, "Exception while polling.", e);
                    return false;
                }
            }
        }, WAIT_TIMEOUT_MS, CHECK_INTERVAL);
    }

    protected String getUrlOnUiThread() throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<String>() {
            @Override
            public String call() throws Exception {
                return mXWalkView.getUrl();
            }
        });
    }

    protected void clearCacheOnUiThread(final boolean includeDiskFiles) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.clearCache(includeDiskFiles);
            }
        });
    }

    protected void clearSingleCacheOnUiThread(final String url) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.clearCacheForSingleFile(url);
            }
        });
    }

    protected String getAPIVersionOnUiThread() throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<String>() {
            @Override
            public String call() throws Exception {
                return mXWalkView.getAPIVersion();
            }
        });
    }

    protected String getXWalkVersionOnUiThread() throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<String>() {
            @Override
            public String call() throws Exception {
                return mXWalkView.getXWalkVersion();
            }
        });
    }

    protected String getCompositingSurfaceTypeOnUiThread() throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<String>() {
            @Override
            public String call() throws Exception {
                return mXWalkView.getCompositingSurfaceType();
            }
        });
    }

    public void clickOnElementId(final String id, String frameName) throws Exception {
        String str;
        if (frameName != null) {
            str = "top.window." + frameName + ".document.getElementById('" + id + "')";
        } else {
            str = "document.getElementById('" + id + "')";
        }
        final String script1 = str + " != null";
        CriteriaHelper.pollInstrumentationThread(new Criteria() {
            @Override
            public boolean isSatisfied() {
                try {
                    String idIsNotNull = executeJavaScriptAndWaitForResult(script1);
                    return idIsNotNull.equals("true");
                } catch (Throwable t) {
                    t.printStackTrace();
                    Assert.fail("Failed to check if DOM is loaded: " + t.toString());
                    return false;
                }
            }
        }, WAIT_TIMEOUT_MS, CHECK_INTERVAL);

        try {
            loadJavaScriptUrl(
                "javascript:var evObj = new MouseEvent('click', {bubbles: true}); "
                        + "document.getElementById('" + id + "').dispatchEvent(evObj);"
                        + "console.log('element with id [" + id + "] clicked');");
        } catch (Throwable t) {
            t.printStackTrace();
        }
    }

    public void simulateKeyAction(final int action) {
        new Thread(new Runnable() {
            public void run() {
                try {
                    getInstrumentation().sendKeySync(new KeyEvent(action,
                            KeyEvent.KEYCODE_DPAD_CENTER));
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }).start();
    }

    protected void stopLoading() throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.stopLoading();
            }
        });
    }

    protected void setAcceptLanguages(final String languages) {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.setAcceptLanguages(languages);
            }
        });
    }

    protected void setUserAgent(final String userAgent) {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.setUserAgentString(userAgent);
            }
        });
    }

    protected String getUserAgent() throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<String>() {
            @Override
            public String call() throws Exception {
                return mXWalkView.getUserAgentString();
            }
        });
    }

    protected void setInitialScale(final int scaleInPercent) {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.setInitialScale(scaleInPercent);
            }
        });
    }

    protected void setOriginAccessWhitelist(final String url, final String[] patterns) {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.setOriginAccessWhitelist(url, patterns);
            }
        });
    }

    protected double getDipScale() {
        return DeviceDisplayInfo.create(mXWalkView.getContext()).getDIPScale();
    }

    protected float getScaleFactor() {
        return getPixelScale() / (float) getDipScale();
    }

    public float getPixelScale() {
        return mTestHelperBridge.getOnScaleChangedHelper().getNewScale();
    }

    protected void ensureScaleBecomes(final float targetScale) throws Throwable {
        pollOnUiThread(new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return targetScale == getScaleFactor();
            }
        });
    }

    abstract class XWalkSettingsTestHelper<T> {
        protected final XWalkView mXWalkViewForHelper;
        protected final XWalkSettings mXWalkSettingsForHelper;

        XWalkSettingsTestHelper(XWalkView view) throws Throwable {
            mXWalkViewForHelper = view;
            mXWalkSettingsForHelper = getXWalkSettingsOnUiThreadByXWalkView(view);
        }

        void ensureSettingHasAlteredValue() throws Throwable {
            ensureSettingHasValue(getAlteredValue());
        }

        void ensureSettingHasInitialValue() throws Throwable {
            ensureSettingHasValue(getInitialValue());
        }

        void setAlteredSettingValue() throws Throwable {
            setCurrentValue(getAlteredValue());
        }

        void setInitialSettingValue() throws Throwable {
            setCurrentValue(getInitialValue());
        }

        protected abstract T getAlteredValue();

        protected abstract T getInitialValue();

        protected abstract T getCurrentValue();

        protected abstract void setCurrentValue(T value) throws Throwable;

        protected abstract void doEnsureSettingHasValue(T value) throws Throwable;

        private void ensureSettingHasValue(T value) throws Throwable {
            assertEquals(value, getCurrentValue());
            doEnsureSettingHasValue(value);
        }
    }

    /**
     * Verifies the following statements about a setting:
     *  - initially, the setting has a default value;
     *  - the setting can be switched to an alternate value and back;
     *  - switching a setting in the first XWalkView doesn't affect the setting
     *    state in the second XWalkView and vice versa.
     *
     * @param helper0 Test helper for the first XWalkView
     * @param helper1 Test helper for the second XWalkView
     * @throws Throwable
     */
    protected void runPerViewSettingsTest(XWalkSettingsTestHelper helper0,
            XWalkSettingsTestHelper helper1) throws Throwable {
        helper0.ensureSettingHasInitialValue();
        helper1.ensureSettingHasInitialValue();
        helper1.setAlteredSettingValue();
        helper0.ensureSettingHasInitialValue();
        helper1.ensureSettingHasAlteredValue();

        helper1.setInitialSettingValue();
        helper0.ensureSettingHasInitialValue();
        helper1.ensureSettingHasInitialValue();

        helper0.setAlteredSettingValue();
        helper0.ensureSettingHasAlteredValue();
        helper1.ensureSettingHasInitialValue();

        helper0.setInitialSettingValue();
        helper0.ensureSettingHasInitialValue();
        helper1.ensureSettingHasInitialValue();

        helper0.setAlteredSettingValue();
        helper0.ensureSettingHasAlteredValue();
        helper1.ensureSettingHasInitialValue();

        helper1.setAlteredSettingValue();
        helper0.ensureSettingHasAlteredValue();
        helper1.ensureSettingHasAlteredValue();

        helper0.setInitialSettingValue();
        helper0.ensureSettingHasInitialValue();
        helper1.ensureSettingHasAlteredValue();

        helper1.setInitialSettingValue();
        helper0.ensureSettingHasInitialValue();
        helper1.ensureSettingHasInitialValue();
    }

    protected Integer getTextZoomOnUiThreadByXWalkView(final XWalkView view) throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<Integer>() {
            @Override
            public Integer call() throws Exception {
                return view.getSettings().getTextZoom();
            }
        });
    }

    protected void setTextZoomOnUiThreadByXWalkView(final int value,
            final XWalkView view) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                view.getSettings().setTextZoom(value);
            }
        });
    }

    protected void poll(final Callable<Boolean> callable) throws Exception {
        CriteriaHelper.pollInstrumentationThread(new Criteria() {
            @Override
            public boolean isSatisfied() {
                try {
                    return callable.call();
                } catch (Throwable e) {
                    Log.e(TAG, "Exception while polling.", e);
                    return false;
                }
            }
        }, WAIT_TIMEOUT_MS, CHECK_INTERVAL);
    }

    static class ViewPair {
        private final XWalkView view0;
        private final TestHelperBridge bridge0;
        private final XWalkView view1;
        private final TestHelperBridge bridge1;

        ViewPair(XWalkView view0, TestHelperBridge bridge0,
                XWalkView view1, TestHelperBridge bridge1) {
            this.view0 = view0;
            this.bridge0 = bridge0;
            this.view1 = view1;
            this.bridge1 = bridge1;
        }

        XWalkView getView0() {
            return view0;
        }

        TestHelperBridge getBridge0() {
            return bridge0;
        }

        XWalkView getView1() {
            return view1;
        }

        TestHelperBridge getBridge1() {
            return bridge1;
        }
    }

    protected ViewPair createViews() throws Throwable {
        TestHelperBridge helperBridge0 = new TestHelperBridge();
        TestHelperBridge helperBridge1 = new TestHelperBridge();
        TestXWalkUIClientBase uiClient0 = new TestXWalkUIClientBase(helperBridge0);
        TestXWalkUIClientBase uiClient1 = new TestXWalkUIClientBase(helperBridge1);
        TestXWalkResourceClientBase resourceClient0 =
                new TestXWalkResourceClientBase(helperBridge0);
        TestXWalkResourceClientBase resourceClient1 =
                new TestXWalkResourceClientBase(helperBridge1);
        ViewPair viewPair =
                createViewsOnMainSync(helperBridge0, helperBridge1, uiClient0, uiClient1,
                        resourceClient0, resourceClient1, getActivity());

        return viewPair;
    }

    protected ViewPair createViewsOnMainSync(final TestHelperBridge helperBridge0,
                                             final TestHelperBridge helperBridge1,
                                             final XWalkUIClient uiClient0,
                                             final XWalkUIClient uiClient1,
                                             final XWalkResourceClient resourceClient0,
                                             final XWalkResourceClient resourceClient1,
                                             final Context context) throws Throwable {
        final XWalkView walkView0 = createXWalkViewContainerOnMainSync(context,
                uiClient0, resourceClient0);
        final XWalkView walkView1 = createXWalkViewContainerOnMainSync(context,
                uiClient1, resourceClient1);
        final AtomicReference<ViewPair> viewPair = new AtomicReference<ViewPair>();

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                viewPair.set(new ViewPair(walkView0, helperBridge0, walkView1, helperBridge1));
            }
        });

        return viewPair.get();
    }

    static void assertFileIsReadable(String filePath) {
        File file = new File(filePath);
        try {
            assertTrue("Test file \"" + filePath + "\" is not readable."
                    + "Please make sure that files from xwalk/test/data/device_files/ "
                    + "has been pushed to the device before testing",
                    file.canRead());
        } catch (SecurityException e) {
            fail("Got a SecurityException for \"" + filePath + "\": " + e.toString());
        }
    }

    protected XWalkSettings getXWalkSettingsOnUiThreadByXWalkView(
            final XWalkView view) throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<XWalkSettings>() {
            @Override
            public XWalkSettings call() throws Exception {
                return view.getSettings();
            }
        });
    }

    protected void loadDataSyncWithXWalkView(final String data,
            final XWalkView view, final TestHelperBridge bridge) throws Exception {
        CallbackHelper pageFinishedHelper = bridge.getOnPageFinishedHelper();
        int currentCallCount = pageFinishedHelper.getCallCount();
        loadDataAsyncWithXWalkView(data, view);
        pageFinishedHelper.waitForCallback(currentCallCount, 1, WAIT_TIMEOUT_SECONDS,
                TimeUnit.SECONDS);
    }

    protected void loadDataAsyncWithXWalkView(final String data,
            final XWalkView view) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                view.loadData(data, "text/html", null);
            }
        });
    }

    protected String executeJavaScriptAndWaitForResultByXWalkView(final String code,
            final XWalkView view, final TestHelperBridge bridge) throws Exception {
        final TestHelperBridge.OnEvaluateJavaScriptResultHelper helper =
                bridge.getOnEvaluateJavaScriptResultHelper();
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                helper.evaluateJavascript(view, code);
            }
        });
        helper.waitUntilHasValue();
        Assert.assertTrue("Failed to retrieve JavaScript evaluation results.", helper.hasValue());
        return helper.getJsonResultAndClear();
    }

    protected void setJavaScriptEnabled(final boolean value) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.getSettings().setJavaScriptEnabled(value);
            }
        });
    }

    protected boolean getJavaScriptEnabled() throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception{
                return mXWalkView.getSettings().getJavaScriptEnabled();
            }
        });
    }

    protected void setAllowSslError(boolean allow) {
        mAllowSslError = allow;
    }

    protected void clearSslPreferences() throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.clearSslPreferences();
            }
        });
    }

    protected void setQuirksMode(final boolean value) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.getSettings().setSupportQuirksMode(value);
            }
        });
    }

    protected void setQuirksModeByXWalkView(final boolean value,
            final XWalkView view) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                view.getSettings().setSupportQuirksMode(value);
            }
        });
    }

    protected void setCacheMode(final int value) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.getSettings().setCacheMode(value);
            }
        });
    }

    protected int getCacheMode() throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<Integer>() {
            @Override
            public Integer call() throws Exception{
                return mXWalkView.getSettings().getCacheMode();
            }
        });
    }

    protected void setBlockNetworkLoads(final boolean value) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.getSettings().setBlockNetworkLoads(value);
            }
        });
    }

    protected void setAllowFileAccess(final boolean value) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.getSettings().setAllowFileAccess(value);
            }
        });
    }

    protected void setAllowUniversalAccessFromFileURLs(final boolean value) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.getSettings().setAllowUniversalAccessFromFileURLs(value);
            }
        });
    }

    protected void setAllowFileAccessFromFileURLs(final boolean value) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.getSettings().setAllowFileAccessFromFileURLs(value);
            }
        });
    }

    protected void setLoadsImagesAutomatically(final boolean value) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.getSettings().setLoadsImagesAutomatically(value);
            }
        });
    }

    protected void setSupportMultipleWindows(final boolean value) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.getSettings().setSupportMultipleWindows(value);
            }
        });
    }

    protected void setJavaScriptCanOpenWindowsAutomatically(final boolean value) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.getSettings().setJavaScriptCanOpenWindowsAutomatically(value);
            }
        });
    }

    protected void setJavaScriptEnabledOnUiThreadByXWalkView(
            final boolean value, final XWalkView view) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                view.getSettings().setJavaScriptEnabled(value);
            }
        });
    }

    protected boolean getJavaScriptEnabledOnUiThreadByXWalkView(
            final XWalkView view) throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return view.getSettings().getJavaScriptEnabled();
            }
        });
    }

    class XWalkSettingsJavaScriptTestHelper extends XWalkSettingsTestHelper<Boolean> {
        private static final String JS_ENABLED_STRING = "JS Enabled";
        private static final String JS_DISABLED_STRING = "JS Disabled";

        XWalkSettingsJavaScriptTestHelper(
                XWalkView xWalkContent,
                final TestHelperBridge helperBridge) throws Throwable {
            super(xWalkContent);
            mView = xWalkContent;
            mHelperBridge = helperBridge;
        }

        @Override
        protected Boolean getAlteredValue() {
            return DISABLED;
        }

        @Override
        protected Boolean getInitialValue() {
            return ENABLED;
        }

        @Override
        protected Boolean getCurrentValue() {
            try {
                return getJavaScriptEnabledOnUiThreadByXWalkView(mView);
            } catch (Exception e) {
                return true;
            }
        }

        @Override
        protected void setCurrentValue(Boolean value) {
            try {
                setJavaScriptEnabledOnUiThreadByXWalkView(value, mView);
            } catch(Exception e) {
            }
        }

        @Override
        protected void doEnsureSettingHasValue(Boolean value) throws Throwable {
            loadDataSyncWithXWalkView(getData(), mView, mHelperBridge);
            assertEquals(
                    value == ENABLED ? JS_ENABLED_STRING : JS_DISABLED_STRING,
                    getTitleOnUiThreadByContent(mView));
        }

        private String getData() {
            return "<html><head><title>" + JS_DISABLED_STRING + "</title>"
                    + "</head><body onload=\"document.title='" + JS_ENABLED_STRING
                    + "';\"></body></html>";
        }

        protected XWalkView mView;
        protected TestHelperBridge mHelperBridge;
    }

    // In contrast to XWalkSettingsJavaScriptTestHelper, doesn't reload the page when testing
    // JavaScript state.
    class XWalkSettingsJavaScriptDynamicTestHelper extends XWalkSettingsJavaScriptTestHelper {
        XWalkSettingsJavaScriptDynamicTestHelper(
                XWalkView xWalkContent,
                final TestHelperBridge helperBridge) throws Throwable {
            super(xWalkContent, helperBridge);
            // Load the page.
            super.doEnsureSettingHasValue(getInitialValue());
        }

        @Override
        protected void doEnsureSettingHasValue(Boolean value) throws Throwable {
            String oldTitle = getTitleOnUiThreadByContent(mView);
            String newTitle = oldTitle + "_modified";
            executeJavaScriptAndWaitForResultByXWalkView(getScript(newTitle), mView, mHelperBridge);
            assertEquals(value == ENABLED ? newTitle : oldTitle, getTitleOnUiThreadByContent(mView));
        }

        private String getScript(String title) {
            return "document.title='" + title + "';";
        }
    }

    protected void setJavaScriptCanOpenWindowsAutomaticallyOnUiThreadByXWalkView(
            final boolean value, final XWalkView view) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                view.getSettings().setJavaScriptCanOpenWindowsAutomatically(value);
            }
        });
    }

    protected boolean getJavaScriptCanOpenWindowsAutomaticallyOnUiThreadByXWalkView(
            final XWalkView view) throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return view.getSettings().getJavaScriptCanOpenWindowsAutomatically();
            }
        });
    }

    class XWalkSettingsJavaScriptPopupsTestHelper extends XWalkSettingsTestHelper<Boolean> {
        private static final String POPUP_ENABLED = "Popup enabled";
        private static final String POPUP_BLOCKED = "Popup blocked";

        XWalkSettingsJavaScriptPopupsTestHelper(
                XWalkView xWalkContent,
                final TestHelperBridge helperBridge) throws Throwable {
            super(xWalkContent);
            mView = xWalkContent;
            mHelperBridge = helperBridge;
        }

        @Override
        protected Boolean getAlteredValue() {
            return DISABLED;
        }

        @Override
        protected Boolean getInitialValue() {
            return ENABLED;
        }

        @Override
        protected Boolean getCurrentValue() {
            try {
                return getJavaScriptCanOpenWindowsAutomaticallyOnUiThreadByXWalkView(mView);
            } catch (Exception e) {
                return true;
            }
        }

        @Override
        protected void setCurrentValue(Boolean value) {
            try {
                setJavaScriptCanOpenWindowsAutomaticallyOnUiThreadByXWalkView(value, mView);
            } catch(Exception e) {
            }
        }

        @Override
        protected void doEnsureSettingHasValue(Boolean value) throws Throwable {
            loadDataSyncWithXWalkView(getData(), mView, mHelperBridge);
            final boolean expectPopupEnabled = value;
            pollInstrumentationThread(new Callable<Boolean>() {
                @Override
                public Boolean call() throws Exception {
                    String title = getTitleOnUiThreadByContent(mView);
                    return expectPopupEnabled ? POPUP_ENABLED.equals(title) :
                            POPUP_BLOCKED.equals(title);
                }
            });
            assertEquals(value ? POPUP_ENABLED : POPUP_BLOCKED, getTitleOnUiThreadByContent(mView));
        }

        private String getData() {
            return "<html><head>"
                    + "<script>"
                    + "    function tryOpenWindow() {"
                    + "        var newWindow = window.open("
                    + "           'data:text/html;charset=utf-8,"
                    + "           <html><head><title>" + POPUP_ENABLED + "</title></head></html>');"
                    + "        if (!newWindow) document.title = '" + POPUP_BLOCKED + "';"
                    + "    }"
                    + "</script></head>"
                    + "<body onload='tryOpenWindow()'></body></html>";
        }

        private XWalkView mView;
        private TestHelperBridge mHelperBridge;
    }

    // This class provides helper methods for testing of settings related to
    // the text autosizing feature.
    abstract class XWalkSettingsTextAutosizingTestHelper<T> extends XWalkSettingsTestHelper<T> {
        protected static final float PARAGRAPH_FONT_SIZE = 14.0f;
        private boolean mNeedToWaitForFontSizeChange;
        private float mOldFontSize;
        XWalkView mView;
        TestHelperBridge mBridge;

        XWalkSettingsTextAutosizingTestHelper(XWalkView view,
                TestHelperBridge bridge) throws Throwable {
            super(view);
            mNeedToWaitForFontSizeChange = false;
            mView = view;
            mBridge = bridge;
            loadDataSyncWithXWalkView(getData(), view, bridge);
        }

        @Override
        protected void setCurrentValue(T value) throws Throwable {
            mNeedToWaitForFontSizeChange = false;
            if (value != getCurrentValue()) {
                mOldFontSize = getActualFontSize();
                mNeedToWaitForFontSizeChange = true;
            }
        }

        protected float getActualFontSize() throws Throwable {
            if (!mNeedToWaitForFontSizeChange) {
                executeJavaScriptAndWaitForResultByXWalkView(
                        "setTitleToActualFontSize()", mView, mBridge);
            } else {
                final float oldFontSize = mOldFontSize;
                poll(new Callable<Boolean>() {
                    @Override
                    public Boolean call() throws Exception {
                        executeJavaScriptAndWaitForResultByXWalkView(
                                "setTitleToActualFontSize()", mView, mBridge);
                        float newFontSize = Float.parseFloat(getTitleOnUiThreadByContent(mView));
                        return newFontSize != oldFontSize;
                    }
                });
                mNeedToWaitForFontSizeChange = false;
            }
            return Float.parseFloat(getTitleOnUiThreadByContent(mView));
        }

        protected String getData() {
            DeviceDisplayInfo deviceInfo = DeviceDisplayInfo.create(mView.getContext());
            int displayWidth = (int) (deviceInfo.getDisplayWidth() / deviceInfo.getDIPScale());
            int layoutWidth = (int) (displayWidth * 2.5f); // Use 2.5 as autosizing layout tests do.
            StringBuilder sb = new StringBuilder();
            sb.append("<html>"
                    + "<head>"
                    + "<meta name=\"viewport\" content=\"width=" + layoutWidth + "\">"
                    + "<style>"
                    + "body { width: " + layoutWidth + "px; margin: 0; overflow-y: hidden; }"
                    + "</style>"
                    + "<script>"
                    + "function setTitleToActualFontSize() {"
                    // parseFloat is used to trim out the "px" suffix.
                    + "  document.title = parseFloat(getComputedStyle("
                    + "    document.getElementById('par')).getPropertyValue('font-size'));"
                    + "}</script></head>"
                    + "<body>"
                    + "<p id=\"par\" style=\"font-size:");
            sb.append(PARAGRAPH_FONT_SIZE);
            sb.append("px;\">");
            // Make the paragraph wide enough for being processed by the font autosizer.
            for (int i = 0; i < 500; i++) {
                sb.append("Hello, World! ");
            }
            sb.append("</p></body></html>");
            return sb.toString();
        }
    }

    protected LayoutAlgorithm getLayoutAlgorithmOnUiThreadByXWalkView(
            final XWalkView view) throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<LayoutAlgorithm>() {
            @Override
            public LayoutAlgorithm call() throws Exception {
                return view.getSettings().getLayoutAlgorithm();
            }
        });
    }

    protected void setLayoutAlgorithmOnUiThreadByXWalkView(final LayoutAlgorithm value,
            final XWalkView view) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                view.getSettings().setLayoutAlgorithm(value);
            }
        });
    }

    protected void setUseWideViewPortOnUiThreadByXWalkView(final boolean value,
            final XWalkView view) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                view.getSettings().setUseWideViewPort(value);
            }
        });
    }

    protected SslCertificate getCertificateOnUiThread() throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<SslCertificate>() {
            @Override
            public SslCertificate call() throws Exception {
                return mXWalkView.getCertificate();
            }
        });
    }

    protected boolean getUseWideViewPortOnUiThreadByXWalkView(
            final XWalkView view) throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return view.getSettings().getUseWideViewPort();
            }
        });
    }

    // To verify whether UseWideViewport works, we check, if the page width specified
    // in the "meta viewport" tag is applied. When UseWideViewport is turned off, the
    // "viewport" tag is ignored, and the layout width is set to device width in DIP pixels.
    // We specify a very high width value to make sure that it doesn't intersect with
    // device screen widths (in DIP pixels).
    class XWalkSettingsUseWideViewportTestHelper extends XWalkSettingsTestHelper<Boolean> {
        private static final String VIEWPORT_TAG_LAYOUT_WIDTH = "3000";
        XWalkView mView;
        TestHelperBridge mBridge;

        XWalkSettingsUseWideViewportTestHelper(XWalkView view,
                TestHelperBridge bridge) throws Throwable {
            super(view);
            mView = view;
            mBridge = bridge;
        }

        @Override
        protected Boolean getAlteredValue() {
            return ENABLED;
        }

        @Override
        protected Boolean getInitialValue() {
            return DISABLED;
        }

        @Override
        protected Boolean getCurrentValue() {
            try {
                return getUseWideViewPortOnUiThreadByXWalkView(mView);
            } catch (Exception e) {
                Log.e(TAG, "Get UseWideViewPort failed.", e);
            }
            return false;
        }

        @Override
        protected void setCurrentValue(Boolean value) throws Throwable {
            setUseWideViewPortOnUiThreadByXWalkView(value, mView);
        }

        @Override
        protected void doEnsureSettingHasValue(Boolean value) throws Throwable {
            loadDataSyncWithXWalkView(getData(), mView, mBridge);
            final String bodyWidth = getTitleOnUiThreadByContent(mView);
            if (value) {
                assertTrue(bodyWidth, VIEWPORT_TAG_LAYOUT_WIDTH.equals(bodyWidth));
            } else {
                assertFalse(bodyWidth, VIEWPORT_TAG_LAYOUT_WIDTH.equals(bodyWidth));
            }
        }

        private String getData() {
            return "<html><head>"
                    + "<meta name='viewport' content='width=" + VIEWPORT_TAG_LAYOUT_WIDTH + "' />"
                    + "</head>"
                    + "<body onload='document.title=document.body.clientWidth'></body></html>";
        }
    }

    private float getScaleFactorByXWalkViewAndHelperBridge(final XWalkView view,
            final TestHelperBridge bridge) {
        final float newScale = bridge.getOnScaleChangedHelper().getNewScale();
        // If new scale is 0.0f, it means the page does not zoom,
        // return the default scale factior: 1.0f.
        if (Float.compare(newScale, 0.0f) == 0) return 1.0f;
        return newScale / (float) DeviceDisplayInfo.create(view.getContext()).getDIPScale();
    }

    protected void setLoadWithOverviewModeOnUiThreadByXWalkView(
            final boolean value, final XWalkView view) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                view.getSettings().setLoadWithOverviewMode(value);
            }
        });
    }

    protected boolean getLoadWithOverviewModeOnUiThreadByXWalkView(
            final XWalkView view) throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return view.getSettings().getLoadWithOverviewMode();
            }
        });
    }

    class XWalkSettingsLoadWithOverviewModeTestHelper extends XWalkSettingsTestHelper<Boolean> {
        private static final float DEFAULT_PAGE_SCALE = 1.0f;
        private final boolean mWithViewPortTag;
        private boolean mExpectScaleChange;
        private int mOnScaleChangedCallCount;
        XWalkView mView;
        TestHelperBridge mBridge;

        XWalkSettingsLoadWithOverviewModeTestHelper(
                XWalkView view,
                TestHelperBridge bridge,
                boolean withViewPortTag) throws Throwable {
            super(view);
            mView = view;
            mBridge = bridge;
            mWithViewPortTag = withViewPortTag;
            setUseWideViewPortOnUiThreadByXWalkView(true, view);
        }

        @Override
        protected Boolean getAlteredValue() {
            return ENABLED;
        }

        @Override
        protected Boolean getInitialValue() {
            return DISABLED;
        }

        @Override
        protected Boolean getCurrentValue() {
            try {
                return getLoadWithOverviewModeOnUiThreadByXWalkView(mView);
            } catch (Exception e) {
                return false;
            }
        }

        @Override
        protected void setCurrentValue(Boolean value) {
            try {
                mExpectScaleChange = getLoadWithOverviewModeOnUiThreadByXWalkView(mView) != value;
                if (mExpectScaleChange)
                    mOnScaleChangedCallCount = mBridge.getOnScaleChangedHelper().getCallCount();
                setLoadWithOverviewModeOnUiThreadByXWalkView(value, mView);
            } catch (Exception e) {
            }
        }

        @Override
        protected void doEnsureSettingHasValue(Boolean value) throws Throwable {
            loadDataSyncWithXWalkView(getData(), mView, mBridge);
            if (mExpectScaleChange) {
                mBridge.getOnScaleChangedHelper().waitForCallback(mOnScaleChangedCallCount);
                mExpectScaleChange = false;
            }

            float currentScale = getScaleFactorByXWalkViewAndHelperBridge(mView, mBridge);
            if (value) {
                assertTrue("Expected: " + currentScale + " < " + DEFAULT_PAGE_SCALE,
                        currentScale < DEFAULT_PAGE_SCALE);
            } else {
                assertEquals(DEFAULT_PAGE_SCALE, currentScale);
            }
        }

        private String getData() {
            return "<html><head>"
                    + (mWithViewPortTag ? "<meta name='viewport' content='width=3000' />" : "")
                    + "</head>"
                    + "<body></body></html>";
        }
    }

    protected boolean hasEnteredFullscreen() throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return mXWalkView.hasEnteredFullscreen();
            }
        });
    }

    protected void leaveFullscreen() throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.leaveFullscreen();
            }
        });
    }

    protected void setDomStorageEnabledOnUiThreadByXWalkView(
            final boolean value, final XWalkView view) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                view.getSettings().setDomStorageEnabled(value);
            }
        });
    }

    protected boolean getDomStorageEnabledOnUiThreadByXWalkView(
            final XWalkView view) throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return view.getSettings().getDomStorageEnabled();
            }
        });
    }

    class XWalkSettingsDomStorageEnabledTestHelper extends XWalkSettingsTestHelper<Boolean> {
        private static final String TEST_FILE = "xwalkview/localStorage.html";
        private static final String NO_LOCAL_STORAGE = "No localStorage";
        private static final String HAS_LOCAL_STORAGE = "Has localStorage";

        XWalkSettingsDomStorageEnabledTestHelper(
                XWalkView xWalkContent,
                final TestHelperBridge helperBridge) throws Throwable {
            super(xWalkContent);
            mView = xWalkContent;
            mHelperBridge = helperBridge;
        }

        @Override
        protected Boolean getAlteredValue() {
            return DISABLED;
        }

        @Override
        protected Boolean getInitialValue() {
            return ENABLED;
        }

        @Override
        protected Boolean getCurrentValue() {
            try {
                return getDomStorageEnabledOnUiThreadByXWalkView(mView);
            } catch (Exception e) {
                return true;
            }
        }

        @Override
        protected void setCurrentValue(Boolean value) {
            try {
                setDomStorageEnabledOnUiThreadByXWalkView(value, mView);
            } catch (Exception e) {
            }
        }

        @Override
        protected void doEnsureSettingHasValue(Boolean value) throws Throwable {
            // It is not permitted to access localStorage from data URLs in WebKit,
            // that is why a standalone page must be used.
            XWalkViewTestBase.assertFileIsReadable(UrlUtils.getTestFilePath(TEST_FILE));
            loadUrlSyncByContent(mView, mHelperBridge,
                    UrlUtils.getTestFileUrl(TEST_FILE));
            assertEquals(
                value == ENABLED ? HAS_LOCAL_STORAGE : NO_LOCAL_STORAGE,
                        getTitleOnUiThreadByContent(mView));
        }

        private XWalkView mView;
        private TestHelperBridge mHelperBridge;
    }

    protected void setDatabaseEnabledOnUiThreadByXWalkView(
            final boolean value, final XWalkView view) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                view.getSettings().setDatabaseEnabled(value);
            }
        });
    }

    protected boolean getDatabaseEnabledOnUiThreadByXWalkView(
            final XWalkView view) throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return view.getSettings().getDatabaseEnabled();
            }
        });
    }

    class XWalkSettingsDatabaseTestHelper extends XWalkSettingsTestHelper<Boolean> {
        private static final String TEST_FILE = "xwalkview/database_access.html";
        private static final String NO_DATABASE = "No database";
        private static final String HAS_DATABASE = "Has database";

        XWalkSettingsDatabaseTestHelper(
                XWalkView xWalkContent,
                final TestHelperBridge helperBridge) throws Throwable {
            super(xWalkContent);
            mView = xWalkContent;
            mHelperBridge = helperBridge;
            XWalkViewTestBase.assertFileIsReadable(UrlUtils.getTestFilePath(TEST_FILE));
        }

        @Override
        protected Boolean getAlteredValue() {
            return DISABLED;
        }

        @Override
        protected Boolean getInitialValue() {
            return ENABLED;
        }

        @Override
        protected Boolean getCurrentValue() {
            try {
                return getDatabaseEnabledOnUiThreadByXWalkView(mView);
            } catch (Exception e) {
                return true;
            }
        }

        @Override
        protected void setCurrentValue(Boolean value) {
            try {
                setDatabaseEnabledOnUiThreadByXWalkView(value, mView);
            } catch (Exception e) {
            }
        }

        @Override
        protected void doEnsureSettingHasValue(Boolean value) throws Throwable {
            // It seems accessing the database through a data scheme is not
            // supported, and fails with a DOM exception (likely a cross-domain
            // violation).
            loadUrlSyncByContent(mView, mHelperBridge,
                    UrlUtils.getTestFileUrl(TEST_FILE));
            assertEquals(
                    value == ENABLED ? HAS_DATABASE : NO_DATABASE,
                    getTitleOnUiThreadByContent(mView));
        }

        private XWalkView mView;
        private TestHelperBridge mHelperBridge;
    }

    protected void setAllowFileAccessOnUiThreadByXWalkView(
            final boolean value, final XWalkView view) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                view.getSettings().setAllowFileAccess(value);
            }
        });
    }

    protected boolean getAllowFileAccessOnUiThreadByXWalkView(
            final XWalkView view) throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return view.getSettings().getAllowFileAccess();
            }
        });
    }

    class XWalkSettingsFileUrlAccessTestHelper extends XWalkSettingsTestHelper<Boolean> {
        private static final String TEST_FILE = "xwalkview/hello_world.html";
        private static final String ACCESS_GRANTED_TITLE = "Hello, World!";

        XWalkSettingsFileUrlAccessTestHelper(
                XWalkView xWalkContent,
                final TestHelperBridge helperBridge,
                int startIndex) throws Throwable {
            super(xWalkContent);
            mView = xWalkContent;
            mHelperBridge = helperBridge;
            mIndex = startIndex;
            XWalkViewTestBase.assertFileIsReadable(UrlUtils.getTestFilePath(TEST_FILE));
        }

        @Override
        protected Boolean getAlteredValue() {
            return DISABLED;
        }

        @Override
        protected Boolean getInitialValue() {
            return ENABLED;
        }

        @Override
        protected Boolean getCurrentValue() {
            try {
                return getAllowFileAccessOnUiThreadByXWalkView(mView);
            } catch (Exception e) {
                return true;
            }
        }

        @Override
        protected void setCurrentValue(Boolean value) {
            try {
                setAllowFileAccessOnUiThreadByXWalkView(value, mView);
            } catch (Exception e) {
            }
        }

        @Override
        protected void doEnsureSettingHasValue(Boolean value) throws Throwable {
            // Use query parameters to avoid hitting a cached page.
            String fileUrl = UrlUtils.getTestFileUrl(TEST_FILE + "?id=" + mIndex);
            mIndex += 2;
            if (value == ENABLED) {
                loadUrlSyncByContent(mView, mHelperBridge, fileUrl);
                assertEquals(ACCESS_GRANTED_TITLE, getTitleOnUiThreadByContent(mView));
            } else {
                loadUrlSyncByContentAndExpectError(mView, mHelperBridge, fileUrl);
            }
        }

        private int mIndex;
        private XWalkView mView;
        private TestHelperBridge mHelperBridge;
    }

    protected void setAllowContentAccessOnUiThreadByXWalkView(
            final boolean value, final XWalkView view) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                view.getSettings().setAllowContentAccess(value);
            }
        });
    }

    protected boolean getAllowContentAccessOnUiThreadByXWalkView(
            final XWalkView view) throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return view.getSettings().getAllowContentAccess();
            }
        });
    }

    class XWalkSettingsContentUrlAccessTestHelper extends XWalkSettingsTestHelper<Boolean> {

        XWalkSettingsContentUrlAccessTestHelper(
                XWalkView xWalkContent,
                final TestHelperBridge helperBridge,
                int startIndex) throws Throwable {
            super(xWalkContent);
            mView = xWalkContent;
            mHelperBridge = helperBridge;
            mTarget = "content_access_" + startIndex;
        }

        @Override
        protected Boolean getAlteredValue() {
            return DISABLED;
        }

        @Override
        protected Boolean getInitialValue() {
            return ENABLED;
        }

        @Override
        protected Boolean getCurrentValue() {
            try {
                return getAllowContentAccessOnUiThreadByXWalkView(mView);
            } catch (Exception e) {
                return true;
            }
        }

        @Override
        protected void setCurrentValue(Boolean value) {
            try {
                setAllowContentAccessOnUiThreadByXWalkView(value, mView);
            } catch (Exception e) {
            }
        }

        @Override
        protected void doEnsureSettingHasValue(Boolean value) throws Throwable {
            XWalkViewTestBase.this.resetResourceRequestCountInContentProvider(mTarget);
            if (value == ENABLED) {
                loadUrlSyncByContent(mView, mHelperBridge, XWalkViewTestBase.this.createContentUrl(mTarget));
                String title = getTitleOnUiThreadByContent(mView);
                assertTrue(title != null);
                assertTrue("[" + mTarget + "] Actual title: \"" + title + "\"",
                        title.contains(mTarget));
                XWalkViewTestBase.this.ensureResourceRequestCountInContentProvider(mTarget, 1);
            } else {
                loadUrlSyncByContentAndExpectError(mView, mHelperBridge,
                        XWalkViewTestBase.this.createContentUrl(mTarget));
                XWalkViewTestBase.this.ensureResourceRequestCountInContentProvider(mTarget, 0);
            }
        }

        private final String mTarget;
        private XWalkView mView;
        private TestHelperBridge mHelperBridge;
    }

    class XWalkSettingsContentUrlAccessFromFileTestHelper extends XWalkSettingsTestHelper<Boolean> {
        private static final String TARGET = "content_from_file";

        XWalkSettingsContentUrlAccessFromFileTestHelper(
                XWalkView xWalkContent,
                final TestHelperBridge helperBridge,
                int startIndex) throws Throwable {
            super(xWalkContent);
            mView = xWalkContent;
            mHelperBridge = helperBridge;
            mIndex = startIndex;
            mTempDir = getInstrumentation().getTargetContext().getCacheDir().getPath();
        }

        @Override
        protected Boolean getAlteredValue() {
            return DISABLED;
        }

        @Override
        protected Boolean getInitialValue() {
            return ENABLED;
        }

        @Override
        protected Boolean getCurrentValue() {
            try {
                return getAllowContentAccessOnUiThreadByXWalkView(mView);
            } catch (Exception e) {
                return true;
            }
        }

        @Override
        protected void setCurrentValue(Boolean value) {
            try {
                setAllowContentAccessOnUiThreadByXWalkView(value, mView);
            } catch (Exception e) {
            }
        }

        @Override
        protected void doEnsureSettingHasValue(Boolean value) throws Throwable {
            XWalkViewTestBase.this.resetResourceRequestCountInContentProvider(TARGET);
            final String fileName = mTempDir + "/" + TARGET + ".html";
            try {
                TestFileUtil.createNewHtmlFile(fileName,
                        TARGET,
                        "<img src=\""
                        // Adding a query avoids hitting a cached image, and also verifies
                        // that content URL query parameters are ignored when accessing
                        // a content provider.
                        + XWalkViewTestBase.this.createContentUrl(TARGET + "?id=" + mIndex) + "\">");
                mIndex += 2;
                loadUrlSyncByContent(mView, mHelperBridge, "file://" + fileName);
                if (value == ENABLED) {
                    XWalkViewTestBase.this.ensureResourceRequestCountInContentProvider(TARGET, 1);
                } else {
                    XWalkViewTestBase.this.ensureResourceRequestCountInContentProvider(TARGET, 0);
                }
            } finally {
                TestFileUtil.deleteFile(fileName);
            }
        }

        private int mIndex;
        private String mTempDir;
        private XWalkView mView;
        private TestHelperBridge mHelperBridge;
    }

    protected void setAllowUniversalAccessFromFileURLsOnUiThreadByXWalkView(
            final boolean value, final XWalkView view) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                view.getSettings().setAllowUniversalAccessFromFileURLs(value);
            }
        });
    }

    protected boolean getAllowUniversalAccessFromFileURLsOnUiThreadByXWalkView(
            final XWalkView view) throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return view.getSettings().getAllowUniversalAccessFromFileURLs();
            }
        });
    }

    class XWalkSettingsUniversalAccessFromFilesTestHelper extends XWalkSettingsTestHelper<Boolean> {
        private static final String TEST_CONTAINER_FILE =
                "xwalkview/iframe_access.html";
        private static final String TEST_FILE = "xwalkview/hello_world.html";
        private static final String ACCESS_DENIED_TITLE = "Exception";

        XWalkSettingsUniversalAccessFromFilesTestHelper(
                XWalkView xWalkContent,
                final TestHelperBridge helperBridge) throws Throwable {
            super(xWalkContent);
            mView = xWalkContent;
            mHelperBridge = helperBridge;
            XWalkViewTestBase.assertFileIsReadable(UrlUtils.getTestFilePath(TEST_CONTAINER_FILE));
            XWalkViewTestBase.assertFileIsReadable(UrlUtils.getTestFilePath(TEST_FILE));

            mIframeContainerUrl = UrlUtils.getTestFileUrl(TEST_CONTAINER_FILE);
            mIframeUrl = UrlUtils.getTestFileUrl(TEST_FILE);
            setAllowUniversalAccessFromFileURLsOnUiThreadByXWalkView(false, mView);
            // If universal access is true, the value of file access doesn't
            // matter. While if universal access is false, having file access
            // enabled will allow file loading.
            setAllowFileAccessFromFileURLsOnUiThreadByXWalkView(false, mView);
        }

        @Override
        protected Boolean getAlteredValue() {
            return ENABLED;
        }

        @Override
        protected Boolean getInitialValue() {
            return DISABLED;
        }

        @Override
        protected Boolean getCurrentValue() {
            try {
                return getAllowUniversalAccessFromFileURLsOnUiThreadByXWalkView(mView);
            } catch (Exception e) {
                return true;
            }
        }

        @Override
        protected void setCurrentValue(Boolean value) {
            try {
                setAllowUniversalAccessFromFileURLsOnUiThreadByXWalkView(value, mView);
            } catch (Exception e) {
            }
        }

        @Override
        protected void doEnsureSettingHasValue(Boolean value) throws Throwable {
            loadUrlSyncByContent(mView, mHelperBridge, mIframeContainerUrl);
            assertEquals(
                    value == ENABLED ? mIframeUrl : ACCESS_DENIED_TITLE,
                    getTitleOnUiThreadByContent(mView));
        }

        private final String mIframeContainerUrl;
        private final String mIframeUrl;
        private XWalkView mView;
        private TestHelperBridge mHelperBridge;
    }

    protected void setAllowFileAccessFromFileURLsOnUiThreadByXWalkView(
            final boolean value, final XWalkView view) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                view.getSettings().setAllowFileAccessFromFileURLs(value);
            }
        });
    }

    protected boolean getAllowFileAccessFromFileURLsOnUiThreadByXWalkView(
            final XWalkView view) throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return view.getSettings().getAllowFileAccessFromFileURLs();
            }
        });
    }

    class XWalkSettingsFileAccessFromFilesIframeTestHelper extends XWalkSettingsTestHelper<Boolean> {
        private static final String TEST_CONTAINER_FILE =
                "xwalkview/iframe_access.html";
        private static final String TEST_FILE = "xwalkview/hello_world.html";
        private static final String ACCESS_DENIED_TITLE = "Exception";

        XWalkSettingsFileAccessFromFilesIframeTestHelper(
                XWalkView xWalkContent,
                final TestHelperBridge helperBridge) throws Throwable {
            super(xWalkContent);
            mView = xWalkContent;
            mHelperBridge = helperBridge;
            XWalkViewTestBase.assertFileIsReadable(UrlUtils.getTestFilePath(TEST_CONTAINER_FILE));
            XWalkViewTestBase.assertFileIsReadable(UrlUtils.getTestFilePath(TEST_FILE));

            mIframeContainerUrl = UrlUtils.getTestFileUrl(TEST_CONTAINER_FILE);
            mIframeUrl = UrlUtils.getTestFileUrl(TEST_FILE);
            setAllowUniversalAccessFromFileURLsOnUiThreadByXWalkView(false, mView);
            setAllowFileAccessFromFileURLsOnUiThreadByXWalkView(false, mView);
        }

        @Override
        protected Boolean getAlteredValue() {
            return ENABLED;
        }

        @Override
        protected Boolean getInitialValue() {
            return DISABLED;
        }

        @Override
        protected Boolean getCurrentValue() {
            try {
                return getAllowFileAccessFromFileURLsOnUiThreadByXWalkView(mView);
            } catch (Exception e) {
                return true;
            }
        }

        @Override
        protected void setCurrentValue(Boolean value) {
            try {
                setAllowFileAccessFromFileURLsOnUiThreadByXWalkView(value, mView);
            } catch (Exception e) {
            }
        }

        @Override
        protected void doEnsureSettingHasValue(Boolean value) throws Throwable {
            loadUrlSyncByContent(mView, mHelperBridge, mIframeContainerUrl);
            assertEquals(
                    value == ENABLED ? mIframeUrl : ACCESS_DENIED_TITLE,
                    getTitleOnUiThreadByContent(mView));
        }

        private final String mIframeContainerUrl;
        private final String mIframeUrl;
        private XWalkView mView;
        private TestHelperBridge mHelperBridge;
    }

    class XWalkSettingsFileAccessFromFilesXhrTestHelper extends XWalkSettingsTestHelper<Boolean> {
        private static final String TEST_FILE = "xwalkview/xhr_access.html";
        private static final String ACCESS_GRANTED_TITLE = "Hello, World!";
        private static final String ACCESS_DENIED_TITLE = "Exception";

        XWalkSettingsFileAccessFromFilesXhrTestHelper(
                XWalkView xWalkContent,
                final TestHelperBridge helperBridge) throws Throwable {
            super(xWalkContent);
            mView = xWalkContent;
            mHelperBridge = helperBridge;
            XWalkViewTestBase.assertFileIsReadable(UrlUtils.getTestFilePath(TEST_FILE));

            mXhrContainerUrl = UrlUtils.getTestFileUrl(TEST_FILE);
            setAllowUniversalAccessFromFileURLsOnUiThreadByXWalkView(false, mView);
            setAllowFileAccessFromFileURLsOnUiThreadByXWalkView(false, mView);
        }

        @Override
        protected Boolean getAlteredValue() {
            return ENABLED;
        }

        @Override
        protected Boolean getInitialValue() {
            return DISABLED;
        }

        @Override
        protected Boolean getCurrentValue() {
            try {
                return getAllowFileAccessFromFileURLsOnUiThreadByXWalkView(mView);
            } catch (Exception e) {
                return true;
            }
        }

        @Override
        protected void setCurrentValue(Boolean value) {
            try {
                setAllowFileAccessFromFileURLsOnUiThreadByXWalkView(value, mView);
            } catch (Exception e) {
            }
        }

        @Override
        protected void doEnsureSettingHasValue(Boolean value) throws Throwable {
            loadUrlSyncByContent(mView, mHelperBridge, mXhrContainerUrl);
            assertEquals(
                    value == ENABLED ? ACCESS_GRANTED_TITLE : ACCESS_DENIED_TITLE,
                    getTitleOnUiThreadByContent(mView));
        }

        private final String mXhrContainerUrl;
        private XWalkView mView;
        private TestHelperBridge mHelperBridge;
    }

    protected void setLoadsImagesAutomaticallyOnUiThreadByXWalkView(
            final boolean value, final XWalkView view) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                view.getSettings().setLoadsImagesAutomatically(value);
            }
        });
    }

    protected boolean getLoadsImagesAutomaticallyOnUiThreadByXWalkView(
            final XWalkView view) throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return view.getSettings().getLoadsImagesAutomatically();
            }
        });
    }

    class XWalkSettingsLoadImagesAutomaticallyTestHelper extends XWalkSettingsTestHelper<Boolean> {

        XWalkSettingsLoadImagesAutomaticallyTestHelper(
                XWalkView xWalkContent,
                final TestHelperBridge helperBridge,
                ImagePageGenerator generator) throws Throwable {
            super(xWalkContent);
            mView = xWalkContent;
            mHelperBridge = helperBridge;
            mGenerator = generator;
        }

        @Override
        protected Boolean getAlteredValue() {
            return DISABLED;
        }

        @Override
        protected Boolean getInitialValue() {
            return ENABLED;
        }

        @Override
        protected Boolean getCurrentValue() {
            try {
                return getLoadsImagesAutomaticallyOnUiThreadByXWalkView(mView);
            } catch (Exception e) {
                return true;
            }
        }

        @Override
        protected void setCurrentValue(Boolean value) {
            try {
                setLoadsImagesAutomaticallyOnUiThreadByXWalkView(value, mView);
            } catch (Exception e) {
            }
        }

        @Override
        protected void doEnsureSettingHasValue(Boolean value) throws Throwable {
            loadDataSyncWithXWalkView(mGenerator.getPageSource(), mView, mHelperBridge);
            assertEquals(value == ENABLED
                    ? ImagePageGenerator.IMAGE_LOADED_STRING
                    : ImagePageGenerator.IMAGE_NOT_LOADED_STRING,
                    getTitleOnUiThreadByContent(mView));
        }

        private XWalkView mView;
        private TestHelperBridge mHelperBridge;
        private ImagePageGenerator mGenerator;
    }

    protected void setBlockNetworkImageOnUiThreadByXWalkView(
            final boolean value, final XWalkView view) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                view.getSettings().setBlockNetworkImage(value);
            }
        });
    }

    protected boolean getBlockNetworkImageOnUiThreadByXWalkView(
            final XWalkView view) throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return view.getSettings().getBlockNetworkImage();
            }
        });
    }

    class XWalkSettingsBlockNetworkImageHelper extends XWalkSettingsTestHelper<Boolean> {

        XWalkSettingsBlockNetworkImageHelper(
                XWalkView xWalkContent,
                final TestHelperBridge helperBridge,
                TestWebServer webServer,
                ImagePageGenerator generator) throws Throwable {
            super(xWalkContent);
            mView = xWalkContent;
            mHelperBridge = helperBridge;
            mWebServer = webServer;
            mGenerator = generator;
        }

        @Override
        protected Boolean getAlteredValue() {
            return ENABLED;
        }

        @Override
        protected Boolean getInitialValue() {
            return DISABLED;
        }

        @Override
        protected Boolean getCurrentValue() {
            try {
                return getBlockNetworkImageOnUiThreadByXWalkView(mView);
            } catch (Exception e) {
                return false;
            }
        }

        @Override
        protected void setCurrentValue(Boolean value) {
            try {
                setBlockNetworkImageOnUiThreadByXWalkView(value, mView);
            } catch (Exception e) {
            }
        }

        @Override
        protected void doEnsureSettingHasValue(Boolean value) throws Throwable {
            final String httpImageUrl = mGenerator.getPageUrl(mWebServer);
            loadUrlSyncByContent(mView, mHelperBridge, httpImageUrl);
            assertEquals(value == DISABLED
                    ? ImagePageGenerator.IMAGE_LOADED_STRING
                    : ImagePageGenerator.IMAGE_NOT_LOADED_STRING,
                    getTitleOnUiThreadByContent(mView));
        }

        private XWalkView mView;
        private TestHelperBridge mHelperBridge;
        private TestWebServer mWebServer;
        private ImagePageGenerator mGenerator;
    }

    /**
     * Verifies the number of resource requests made to the content provider.
     * @param resource Resource name
     * @param expectedCount Expected resource requests count
     */
    private void ensureResourceRequestCountInContentProvider(String resource, int expectedCount) {
        Context context = getInstrumentation().getTargetContext();
        int actualCount = TestContentProvider.getResourceRequestCount(context, resource);
        assertEquals(expectedCount, actualCount);
    }

    private void resetResourceRequestCountInContentProvider(String resource) {
        Context context = getInstrumentation().getTargetContext();
        TestContentProvider.resetResourceRequestCount(context, resource);
    }

    private String createContentUrl(final String target) {
        return TestContentProvider.createContentUrl(target);
    }
}
