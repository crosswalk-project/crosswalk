// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.net.http.SslError;
import android.net.Uri;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.view.KeyEvent;
import android.webkit.ValueCallback;
import android.webkit.WebResourceResponse;

import java.io.InputStream;
import java.io.IOException;
import java.util.concurrent.atomic.AtomicReference;
import java.util.concurrent.Callable;
import java.util.concurrent.FutureTask;
import java.util.concurrent.TimeUnit;

import junit.framework.Assert;

import org.chromium.content_public.browser.LoadUrlParams;
import org.chromium.content.browser.test.util.CallbackHelper;
import org.chromium.content.browser.test.util.Criteria;
import org.chromium.content.browser.test.util.CriteriaHelper;
import org.chromium.ui.gfx.DeviceDisplayInfo;

import org.xwalk.core.ClientCertRequest;
import org.xwalk.core.XWalkDownloadListener;
import org.xwalk.core.XWalkHttpAuthHandler;
import org.xwalk.core.XWalkJavascriptResult;
import org.xwalk.core.XWalkNavigationHistory;
import org.xwalk.core.XWalkNavigationItem;
import org.xwalk.core.XWalkResourceClient;
import org.xwalk.core.XWalkSettings;
import org.xwalk.core.XWalkUIClient;
import org.xwalk.core.XWalkView;
import org.xwalk.core.XWalkWebResourceRequest;
import org.xwalk.core.XWalkWebResourceResponse;

public class XWalkViewTestBase
       extends ActivityInstrumentationTestCase2<XWalkViewTestRunnerActivity> {
    protected final static int WAIT_TIMEOUT_SECONDS = 15;
    protected final static long WAIT_TIMEOUT_MS = 2000;
    private final static int CHECK_INTERVAL = 100;
    private final static String TAG = "XWalkViewTestBase";
    private XWalkView mXWalkView;
    private boolean mAllowSslError = true;
    final TestHelperBridge mTestHelperBridge = new TestHelperBridge();

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

    protected void loadUrlAsync(final String url) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.load(url, null);
            }
        });
    }

    protected void loadDataSync(final String url, final String data, final String mimeType,
            final boolean isBase64Encoded) throws Exception {
        CallbackHelper pageFinishedHelper = mTestHelperBridge.getOnPageFinishedHelper();
        int currentCallCount = pageFinishedHelper.getCallCount();
        loadDataAsync(url, data, mimeType, isBase64Encoded);
        pageFinishedHelper.waitForCallback(currentCallCount, 1, WAIT_TIMEOUT_SECONDS,
                TimeUnit.SECONDS);
    }

    protected void loadDataAsync(final String url, final String data, final String mimeType,
             final boolean isBase64Encoded) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.load(url, data);
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

    protected void loadUrlAsyncByContent(final XWalkView xWalkContent,
            final String url) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                xWalkContent.load(url, null);
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
        loadDataSync(fileName, fileContent, "text/html", false);
    }

    public void loadAssetFileAndWaitForTitle(String fileName) throws Exception {
        CallbackHelper getTitleHelper = mTestHelperBridge.getOnTitleUpdatedHelper();
        int currentCallCount = getTitleHelper.getCallCount();
        String fileContent = getFileContent(fileName);

        loadDataAsync(fileName, fileContent, "text/html", false);

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

    public void clickOnElementId(final String id, String frameName) throws Exception {
        String str;
        if (frameName != null) {
            str = "top.window." + frameName + ".document.getElementById('" + id + "')";
        } else {
            str = "document.getElementById('" + id + "')";
        }
        final String script1 = str + " != null";
        final String script2 = str + ".dispatchEvent(evObj);";
        CriteriaHelper.pollForCriteria(new Criteria() {
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
            loadJavaScriptUrl("javascript:var evObj = document.createEvent('Events'); " +
                "evObj.initEvent('click', true, false); " +
                script2 +
                "console.log('element with id [" + id + "] clicked');");
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
        CriteriaHelper.pollForCriteria(new Criteria() {
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
                view.load(null, data);
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
}
