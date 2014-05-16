// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.app.Activity;
import android.content.Context;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.webkit.WebResourceResponse;

import java.io.InputStream;
import java.io.IOException;
import java.util.concurrent.atomic.AtomicReference;
import java.util.concurrent.Callable;
import java.util.concurrent.FutureTask;
import java.util.concurrent.TimeUnit;

import junit.framework.Assert;

import org.chromium.content.browser.LoadUrlParams;
import org.chromium.content.browser.test.util.CallbackHelper;
import org.chromium.content.browser.test.util.Criteria;
import org.chromium.content.browser.test.util.CriteriaHelper;

import org.xwalk.core.XWalkClient;
import org.xwalk.core.XWalkNavigationHistory;
import org.xwalk.core.XWalkResourceClient;
import org.xwalk.core.XWalkSettings;
import org.xwalk.core.XWalkView;
import org.xwalk.core.XWalkWebChromeClient;

public class XWalkViewTestBase
       extends ActivityInstrumentationTestCase2<XWalkViewTestRunnerActivity> {
    protected final static int WAIT_TIMEOUT_SECONDS = 15;
    private final static String TAG = "XWalkViewTestBase";
    private XWalkView mXWalkView;
    final TestHelperBridge mTestHelperBridge = new TestHelperBridge();

    class TestXWalkClientBase extends XWalkClient {
        TestHelperBridge mInnerContentsClient;
        public TestXWalkClientBase(TestHelperBridge client) {
            super(getXWalkView());
            mInnerContentsClient = client;
        }

        @Override
        public void onPageStarted(XWalkView view, String url) {
            mInnerContentsClient.onPageStarted(url);
        }

        @Override
        public void onPageFinished(XWalkView view, String url) {
            mInnerContentsClient.onPageFinished(url);
        }
    }

    class TestXWalkClient extends TestXWalkClientBase {
        public TestXWalkClient() {
            super(mTestHelperBridge);
        }
    }

    class TestXWalkWebChromeClientBase extends XWalkWebChromeClient {
        TestHelperBridge mInnerContentsClient;
        public TestXWalkWebChromeClientBase(TestHelperBridge client) {
            super(getXWalkView());
            mInnerContentsClient = client;
        }

        @Override
        public void onReceivedTitle(XWalkView view, String title) {
            mInnerContentsClient.onTitleChanged(title);
        }
    }

    class TestXWalkWebChromeClient extends TestXWalkWebChromeClientBase {
        public TestXWalkWebChromeClient() {
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
            mTestHelperBridge.onLoadStarted(url);
        }

        @Override
        public void onReceivedLoadError(XWalkView view, int errorCode, String description, String failingUrl) {
            mTestHelperBridge.onReceivedLoadError(errorCode, description, failingUrl);
        }

        @Override
        public WebResourceResponse shouldInterceptLoadRequest(XWalkView view,
                String url) {
            return mTestHelperBridge.shouldInterceptLoadRequest(url);
        }
    }

    class TestXWalkResourceClient extends TestXWalkResourceClientBase {
        public TestXWalkResourceClient() {
            super(mTestHelperBridge);
        }
    }

    void setXWalkClient(final XWalkClient client) {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getXWalkView().setXWalkClient(client);
            }
        });
    }

    void setXWalkWebChromeClient(final XWalkWebChromeClient client) {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getXWalkView().setXWalkWebChromeClient(client);
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

    static class ViewPair {
        private final XWalkView content0;
        private final TestHelperBridge client0;
        private final XWalkView content1;
        private final TestHelperBridge client1;

        ViewPair(XWalkView content0, TestHelperBridge client0,
                XWalkView content1, TestHelperBridge client1) {
            this.content0 = content0;
            this.client0 = client0;
            this.content1 = content1;
            this.client1 = client1;
        }

        XWalkView getContent0() {
            return content0;
        }

        TestHelperBridge getClient0() {
            return client0;
        }

        XWalkView getContent1() {
            return content1;
        }

        TestHelperBridge getClient1() {
            return client1;
        }
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

    protected XWalkSettings getXWalkSettingsOnUiThreadByContent(
            final XWalkView xwalkContent) throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<XWalkSettings>() {
            @Override
            public XWalkSettings call() throws Exception {
                return xwalkContent.getSettings();
            }
        });
    }

    protected XWalkView createXWalkViewContainerOnMainSync(
            final Context context,
            final XWalkClient client,
            final XWalkResourceClient resourceClient,
            final XWalkWebChromeClient webChromeClient) throws Exception {
        final AtomicReference<XWalkView> xWalkViewContainer =
                new AtomicReference<XWalkView>();
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                xWalkViewContainer.set(new XWalkView(context, getActivity()));
                getActivity().addView(xWalkViewContainer.get());
                xWalkViewContainer.get().setXWalkClient(client);
                xWalkViewContainer.get().setResourceClient(resourceClient);
                xWalkViewContainer.get().setXWalkWebChromeClient(webChromeClient);
            }
        });

        return xWalkViewContainer.get();
    }

    protected ViewPair createViewsOnMainSync(final TestHelperBridge helperBridge0,
                                             final TestHelperBridge helperBridge1,
                                             final XWalkClient client0,
                                             final XWalkClient client1,
                                             final XWalkResourceClient resourceClient0,
                                             final XWalkResourceClient resourceClient1,
                                             final XWalkWebChromeClient chromeClient0,
                                             final XWalkWebChromeClient chromeClient1,
                                             final Context context) throws Throwable {
        final XWalkView walkView0 = createXWalkViewContainerOnMainSync(context,
                client0, resourceClient0, chromeClient0);
        final XWalkView walkView1 = createXWalkViewContainerOnMainSync(context,
                client1, resourceClient1, chromeClient1);
        final AtomicReference<ViewPair> viewPair = new AtomicReference<ViewPair>();

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                viewPair.set(new ViewPair(walkView0, helperBridge0, walkView1, helperBridge1));
            }
        });

        return viewPair.get();
    }

    protected void loadAssetFile(String fileName) throws Exception {
        String fileContent = getFileContent(fileName);
        loadDataSync(fileName, fileContent, "text/html", false);
    }

    public void loadAssetFileAndWaitForTitle(String fileName) throws Exception {
        CallbackHelper getTitleHelper = mTestHelperBridge.getOnTitleUpdatedHelper();
        int currentCallCount = getTitleHelper.getCallCount();
        String fileContent = getFileContent(fileName);

        loadDataSync(fileName, fileContent, "text/html", false);

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

    protected ViewPair createViews() throws Throwable {
        TestHelperBridge helperBridge0 = new TestHelperBridge();
        TestHelperBridge helperBridge1 = new TestHelperBridge();
        TestXWalkClientBase viewClient0 = new TestXWalkClientBase(helperBridge0);
        TestXWalkClientBase viewClient1 = new TestXWalkClientBase(helperBridge1);
        TestXWalkWebChromeClientBase chromeClient0 =
                new TestXWalkWebChromeClientBase(helperBridge0);
        TestXWalkWebChromeClientBase chromeClient1 =
                new TestXWalkWebChromeClientBase(helperBridge1);
        TestXWalkResourceClientBase resourceClient0 =
                new TestXWalkResourceClientBase(helperBridge0);
        TestXWalkResourceClientBase resourceClient1 =
                new TestXWalkResourceClientBase(helperBridge1);
        ViewPair viewPair =
                createViewsOnMainSync(helperBridge0, helperBridge1, viewClient0, viewClient1,
                        resourceClient0, resourceClient1, chromeClient0, chromeClient1,
                                getActivity());

        return viewPair;
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
}
