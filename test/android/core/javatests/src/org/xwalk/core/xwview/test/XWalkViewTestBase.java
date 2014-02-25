// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.app.Activity;
import android.content.Context;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;

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
import org.chromium.content.browser.test.util.TestCallbackHelperContainer.OnEvaluateJavaScriptResultHelper;
import org.xwalk.core.XWalkClient;
import org.xwalk.core.XWalkContent;
import org.xwalk.core.XWalkContentsClient;
import org.xwalk.core.XWalkSettings;
import org.xwalk.core.XWalkView;
import org.xwalk.core.XWalkWebChromeClient;

public class XWalkViewTestBase
       extends ActivityInstrumentationTestCase2<XWalkViewTestRunnerActivity> {
    protected final static int WAIT_TIMEOUT_SECONDS = 15;
    private final static String TAG = "XWalkViewTestBase";
    private XWalkView mXWalkView;
    final TestXWalkViewContentsClient mTestContentsClient = new TestXWalkViewContentsClient();

    static class ViewPair {
        private final XWalkContent content0;
        private final TestXWalkViewContentsClient client0;
        private final XWalkContent content1;
        private final TestXWalkViewContentsClient client1;

        ViewPair(XWalkContent content0, TestXWalkViewContentsClient client0,
                XWalkContent content1, TestXWalkViewContentsClient client1) {
            this.content0 = content0;
            this.client0 = client0;
            this.content1 = content1;
            this.client1 = client1;
        }

        XWalkContent getContent0() {
            return content0;
        }

        TestXWalkViewContentsClient getClient0() {
            return client0;
        }

        XWalkContent getContent1() {
            return content1;
        }

        TestXWalkViewContentsClient getClient1() {
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
                mXWalkView.getXWalkViewContentForTest().installWebContentsObserverForTest(mTestContentsClient);
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
        CallbackHelper pageFinishedHelper = mTestContentsClient.getOnPageFinishedHelper();
        int currentCallCount = pageFinishedHelper.getCallCount();
        loadUrlAsync(url);

        pageFinishedHelper.waitForCallback(currentCallCount, 1, WAIT_TIMEOUT_SECONDS,
                TimeUnit.SECONDS);
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
        CallbackHelper pageFinishedHelper = mTestContentsClient.getOnPageFinishedHelper();
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
                mXWalkView.getXWalkViewContentForTest().getContentViewCoreForTest(
                        ).loadUrl(LoadUrlParams.createLoadDataParams(
                        data, mimeType, isBase64Encoded));
            }
        });
    }

    protected void loadUrlSyncByContent(final XWalkContent xWalkContent,
            final TestXWalkViewContentsClient contentsClient,
            final String url) throws Exception {
        CallbackHelper pageFinishedHelper = contentsClient.getOnPageFinishedHelper();
        int currentCallCount = pageFinishedHelper.getCallCount();
        loadUrlAsyncByContent(xWalkContent, url);

        pageFinishedHelper.waitForCallback(currentCallCount, 1, WAIT_TIMEOUT_SECONDS,
                TimeUnit.SECONDS);
    }

    protected void loadUrlAsyncByContent(final XWalkContent xWalkContent,
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

    protected String getTitleOnUiThreadByContent(final XWalkContent xWalkContent) throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<String>() {
            @Override
            public String call() throws Exception {
                String title = xWalkContent.getContentViewCoreForTest().getTitle();
                return title;
            }
        });
    }

    protected XWalkSettings getXWalkSettingsOnUiThreadByContent(
            final XWalkContent xwalkContent) throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<XWalkSettings>() {
            @Override
            public XWalkSettings call() throws Exception {
                return xwalkContent.getSettings();
            }
        });
    }

    protected XWalkView createXWalkViewContainerOnMainSync(
            final Context context,
            final XWalkContentsClient contentClient,
            final XWalkClient xWalkClient,
            final XWalkWebChromeClient xWlkWebChromeClient) throws Exception {
        final AtomicReference<XWalkView> xWalkViewContainer =
                new AtomicReference<XWalkView>();
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                xWalkViewContainer.set(new XWalkView(context, getActivity()));
                getActivity().addView(xWalkViewContainer.get());
                xWalkViewContainer.get().getXWalkViewContentForTest().installWebContentsObserverForTest(contentClient);
                xWalkViewContainer.get().setXWalkClient(xWalkClient);
                xWalkViewContainer.get().setXWalkWebChromeClient(xWlkWebChromeClient);
            }
        });

        return xWalkViewContainer.get();
    }

    protected XWalkContent getXWalkContentOnMainSync(final XWalkView view) throws Exception {
        final AtomicReference<XWalkContent> xWalkContent =
                new AtomicReference<XWalkContent>();
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                xWalkContent.set(view.getXWalkViewContentForTest());
            }
        });

        return xWalkContent.get();
    }

    protected ViewPair createViewsOnMainSync(final TestXWalkViewContentsClient contentClient0,
            final TestXWalkViewContentsClient contentClient1, final XWalkClient viewClient0,
                    final XWalkClient viewClient1, final XWalkWebChromeClient chromeClient0,
                            final XWalkWebChromeClient chromeClient1, final Context context) throws Throwable {
        final XWalkView walkView0 = createXWalkViewContainerOnMainSync(context, contentClient0,
                viewClient0, chromeClient0);
        final XWalkView walkView1 = createXWalkViewContainerOnMainSync(context, contentClient1,
                viewClient1, chromeClient1);
        final AtomicReference<ViewPair> viewPair = new AtomicReference<ViewPair>();

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                XWalkContent content0 = walkView0.getXWalkViewContentForTest();
                XWalkContent content1 = walkView1.getXWalkViewContentForTest();
                viewPair.set(new ViewPair(content0, contentClient0, content1, contentClient1));
            }
        });

        return viewPair.get();
    }

    protected void loadAssetFile(String fileName) throws Exception {
        String fileContent = getFileContent(fileName);
        loadDataSync(fileContent, "text/html", false);
    }

    public void loadAssetFileAndWaitForTitle(String fileName) throws Exception {
        CallbackHelper getTitleHelper = mTestContentsClient.getOnTitleUpdatedHelper();
        int currentCallCount = getTitleHelper.getCallCount();
        String fileContent = getFileContent(fileName);

        loadDataSync(fileContent, "text/html", false);

        getTitleHelper.waitForCallback(currentCallCount, 1, WAIT_TIMEOUT_SECONDS,
                TimeUnit.SECONDS);
    }

    protected XWalkView getXWalkView() {
        return mXWalkView;
    }

    protected void runTestWaitPageFinished(Runnable runnable) throws Exception{
        CallbackHelper pageFinishedHelper = mTestContentsClient.getOnPageFinishedHelper();
        int currentCallCount = pageFinishedHelper.getCallCount();
        runnable.run();
        pageFinishedHelper.waitForCallback(currentCallCount, 1, WAIT_TIMEOUT_SECONDS,
                TimeUnit.SECONDS);
    }

    protected void reloadSync() throws Exception {
        runTestWaitPageFinished(new Runnable(){
            @Override
            public void run() {
                getInstrumentation().runOnMainSync(new Runnable() {
                    @Override
                    public void run() {
                        mXWalkView.reload();
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
                        mXWalkView.goBack();
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
                        mXWalkView.goForward();
                    }
                });
            }
        });
    }

    protected void clearHistoryOnUiThread() throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView.clearHistory();
            }
        });
    }

    protected boolean canGoBackOnUiThread() throws Throwable {
        return runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() {
                return mXWalkView.canGoBack();
            }
        });
    }

    protected boolean canGoForwardOnUiThread() throws Throwable {
        return runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() {
                return mXWalkView.canGoForward();
            }
        });
    }

    protected String executeJavaScriptAndWaitForResult(final String code) throws Exception {
        final OnEvaluateJavaScriptResultHelper helper = mTestContentsClient.getOnEvaluateJavaScriptResultHelper();
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                XWalkContent content = mXWalkView.getXWalkViewContentForTest();
                helper.evaluateJavaScript(content.getContentViewCoreForTest(), code);
            }
        });
        helper.waitUntilHasValue();
        Assert.assertTrue("Failed to retrieve JavaScript evaluation results.", helper.hasValue());
        return helper.getJsonResultAndClear();
    }
}
