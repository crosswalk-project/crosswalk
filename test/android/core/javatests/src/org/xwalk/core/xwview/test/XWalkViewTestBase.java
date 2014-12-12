// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.app.Activity;
import android.content.Context;
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

import org.xwalk.core.XWalkJavascriptResult;
import org.xwalk.core.XWalkNavigationHistory;
import org.xwalk.core.XWalkNavigationItem;
import org.xwalk.core.XWalkResourceClient;
import org.xwalk.core.XWalkUIClient;
import org.xwalk.core.XWalkView;

public class XWalkViewTestBase
       extends ActivityInstrumentationTestCase2<XWalkViewTestRunnerActivity> {
    protected final static int WAIT_TIMEOUT_SECONDS = 15;
    protected final static long WAIT_TIMEOUT_MS = 2000;
    private final static int CHECK_INTERVAL = 100;
    private final static String TAG = "XWalkViewTestBase";
    private XWalkView mXWalkView;
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
            mInnerContentsClient.onScaleChanged(newScale);
        }

        @Override
        public void onRequestFocus(XWalkView view) {
            mInnerContentsClient.onRequestFocus();
        }

        @Override
        public boolean onJavascriptModalDialog(XWalkView view,
                XWalkUIClient.JavascriptMessageType type, String url, String message,
                        String defaultValue, XWalkJavascriptResult result) {
            return mInnerContentsClient.onJavascriptModalDialog(message);
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
        public WebResourceResponse shouldInterceptLoadRequest(XWalkView view,
                String url) {
            return mInnerContentsClient.shouldInterceptLoadRequest(url);
        }

        @Override
        public void onProgressChanged(XWalkView view, int progressInPercent) {
            mTestHelperBridge.onProgressChanged(progressInPercent);
        }

        @Override
        public boolean shouldOverrideUrlLoading(XWalkView view, String url) {
            return mTestHelperBridge.shouldOverrideUrlLoading(url);
        }
    }

    class TestXWalkResourceClient extends TestXWalkResourceClientBase {
        public TestXWalkResourceClient() {
            super(mTestHelperBridge);
        }
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

    protected boolean pollOnUiThread(final Callable<Boolean> callable) throws Exception {
        return CriteriaHelper.pollForCriteria(new Criteria() {
            @Override
            public boolean isSatisfied() {
                try {
                    return runTestOnUiThreadAndGetResult(callable);
                } catch (Throwable e) {
                    return false;
                }
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
                getInstrumentation().runOnMainSync(new Runnable() {
                    @Override
                    public void run() {
                        mXWalkView.getNavigationHistory().navigate(
                            XWalkNavigationHistory.Direction.BACKWARD, 1);
                    }
                });
            }
        });
    }

    protected void goForwardSync() throws Throwable {
        runTestWaitPageFinished(new Runnable(){
            @Override
            public void run() {
                getInstrumentation().runOnMainSync(new Runnable() {
                    @Override
                    public void run() {
                        mXWalkView.getNavigationHistory().navigate(
                            XWalkNavigationHistory.Direction.FORWARD, 1);
                    }
                });
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
        Assert.assertTrue(CriteriaHelper.pollForCriteria(new Criteria() {
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
        }, WAIT_TIMEOUT_MS, CHECK_INTERVAL));

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
}
