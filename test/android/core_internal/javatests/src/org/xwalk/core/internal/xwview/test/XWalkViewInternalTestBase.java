// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.xwview.test;

import android.app.Activity;
import android.content.Context;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.webkit.WebResourceResponse;
import android.widget.FrameLayout;

import java.io.InputStream;
import java.io.IOException;
import java.util.concurrent.atomic.AtomicReference;
import java.util.concurrent.Callable;
import java.util.concurrent.FutureTask;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.TimeUnit;

import junit.framework.Assert;

import org.chromium.content.browser.ContentViewCore;
import org.chromium.content.browser.test.util.CallbackHelper;
import org.chromium.content.browser.test.util.Criteria;
import org.chromium.content.browser.test.util.CriteriaHelper;

import org.xwalk.core.internal.XWalkNavigationHistoryInternal;
import org.xwalk.core.internal.XWalkNavigationItemInternal;
import org.xwalk.core.internal.XWalkResourceClientInternal;
import org.xwalk.core.internal.XWalkSettings;
import org.xwalk.core.internal.XWalkUIClientInternal;
import org.xwalk.core.internal.XWalkViewInternal;
import org.xwalk.core.internal.XWalkWebChromeClient;

public class XWalkViewInternalTestBase
       extends ActivityInstrumentationTestCase2<XWalkViewInternalTestRunnerActivity> {
    protected final static int WAIT_TIMEOUT_SECONDS = 15;
    private final static String TAG = "XWalkViewInternalTestBase";
    private XWalkViewInternal mXWalkViewInternal;
    final TestHelperBridge mTestHelperBridge = new TestHelperBridge();

    class TestXWalkUIClientInternalBase extends XWalkUIClientInternal {
        TestHelperBridge mInnerContentsClient;
        public TestXWalkUIClientInternalBase(TestHelperBridge client) {
            super(getXWalkView());
            mInnerContentsClient = client;
        }

        @Override
        public void onPageLoadStarted(XWalkViewInternal view, String url) {
            mInnerContentsClient.onPageStarted(url);
        }

        @Override
        public void onPageLoadStopped(XWalkViewInternal view, String url, LoadStatusInternal status) {
            mInnerContentsClient.onPageFinished(url);
        }

        @Override
        public void onReceivedTitle(XWalkViewInternal view, String title) {
            mInnerContentsClient.onTitleChanged(title);
        }
    }

    class TestXWalkUIClientInternal extends TestXWalkUIClientInternalBase {
        public TestXWalkUIClientInternal() {
            super(mTestHelperBridge);
        }
    }

    class TestXWalkResourceClientBase extends XWalkResourceClientInternal {
        TestHelperBridge mInnerContentsClient;
        public TestXWalkResourceClientBase(TestHelperBridge client) {
            super(mXWalkViewInternal);
            mInnerContentsClient = client;
        }

        @Override
        public void onLoadStarted(XWalkViewInternal view, String url) {
            mInnerContentsClient.onLoadStarted(url);
        }

        @Override
        public void onReceivedLoadError(XWalkViewInternal view, int errorCode,
                String description, String failingUrl) {
            mInnerContentsClient.onReceivedLoadError(errorCode, description, failingUrl);
        }

        @Override
        public WebResourceResponse shouldInterceptLoadRequest(XWalkViewInternal view,
                String url) {
            return mInnerContentsClient.shouldInterceptLoadRequest(url);
        }
    }

    class TestXWalkResourceClient extends TestXWalkResourceClientBase {
        public TestXWalkResourceClient() {
            super(mTestHelperBridge);
        }
    }

    class TestXWalkWebChromeClientBase extends XWalkWebChromeClient {
        private CallbackHelper mOnShowCustomViewCallbackHelper = new CallbackHelper();
        private CallbackHelper mOnHideCustomViewCallbackHelper = new CallbackHelper();

        private Activity mActivity = getActivity();
        private View mCustomView;
        private XWalkWebChromeClient.CustomViewCallback mExitCallback;

        public TestXWalkWebChromeClientBase() {
            super(mXWalkViewInternal);
        }

        @Override
        public void onShowCustomView(View view, XWalkWebChromeClient.CustomViewCallback callback) {
            mCustomView = view;
            mExitCallback = callback;
            mActivity.getWindow().setFlags(
                    WindowManager.LayoutParams.FLAG_FULLSCREEN,
                    WindowManager.LayoutParams.FLAG_FULLSCREEN);

            mActivity.getWindow().addContentView(view,
                    new FrameLayout.LayoutParams(
                            ViewGroup.LayoutParams.MATCH_PARENT,
                            ViewGroup.LayoutParams.MATCH_PARENT,
                            Gravity.CENTER));
            mOnShowCustomViewCallbackHelper.notifyCalled();
        }

        @Override
        public void onHideCustomView() {
            mActivity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
            mOnHideCustomViewCallbackHelper.notifyCalled();
        }

        public XWalkWebChromeClient.CustomViewCallback getExitCallback() {
            return mExitCallback;
        }

        public View getCustomView() {
            return mCustomView;
        }

        public boolean wasCustomViewShownCalled() {
            return mOnShowCustomViewCallbackHelper.getCallCount() > 0;
        }

        public void waitForCustomViewShown() throws TimeoutException, InterruptedException {
            mOnShowCustomViewCallbackHelper.waitForCallback(0, 1, WAIT_TIMEOUT_SECONDS, TimeUnit.SECONDS);
        }

        public void waitForCustomViewHidden() throws InterruptedException, TimeoutException {
            mOnHideCustomViewCallbackHelper.waitForCallback(0, 1, WAIT_TIMEOUT_SECONDS, TimeUnit.SECONDS);
        }
    }

    void setUIClient(final XWalkUIClientInternal client) {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getXWalkView().setUIClient(client);
            }
        });
    }

    void setResourceClient(final XWalkResourceClientInternal client) {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getXWalkView().setResourceClient(client);
            }
        });
    }

    void setXWalkWebChromeClient(final TestXWalkWebChromeClientBase client) {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkViewInternal.setXWalkWebChromeClient(client);
            }
        });
    }

    static class ViewPair {
        private final XWalkViewInternal view0;
        private final TestHelperBridge client0;
        private final XWalkViewInternal view1;
        private final TestHelperBridge client1;

        ViewPair(XWalkViewInternal view0, TestHelperBridge client0,
                XWalkViewInternal view1, TestHelperBridge client1) {
            this.view0 = view0;
            this.client0 = client0;
            this.view1 = view1;
            this.client1 = client1;
        }

        XWalkViewInternal getView0() {
            return view0;
        }

        TestHelperBridge getClient0() {
            return client0;
        }

        XWalkViewInternal getView1() {
            return view1;
        }

        TestHelperBridge getClient1() {
            return client1;
        }
    }

    public XWalkViewInternalTestBase() {
        super(XWalkViewInternalTestRunnerActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        // Must call getActivity() here but not in main thread.
        final Activity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkViewInternal = new XWalkViewInternal(activity, activity);
                getActivity().addView(mXWalkViewInternal);
                mXWalkViewInternal.setUIClient(new TestXWalkUIClientInternal());
                mXWalkViewInternal.setResourceClient(new TestXWalkResourceClient());
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
                mXWalkViewInternal.load(url, null);
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
                mXWalkViewInternal.load(url, data);
            }
        });
    }

    protected void loadUrlSyncByContent(final XWalkViewInternal xWalkViewInternal,
            final TestHelperBridge contentsClient,
            final String url) throws Exception {
        CallbackHelper pageFinishedHelper = contentsClient.getOnPageFinishedHelper();
        int currentCallCount = pageFinishedHelper.getCallCount();
        loadUrlAsyncByContent(xWalkViewInternal, url);

        pageFinishedHelper.waitForCallback(currentCallCount, 1, WAIT_TIMEOUT_SECONDS,
                TimeUnit.SECONDS);
    }

    protected void loadUrlAsyncByContent(final XWalkViewInternal xWalkViewInternal,
            final String url) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                xWalkViewInternal.load(url, null);
            }
        });
    }

    protected String getTitleOnUiThread() throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<String>() {
            @Override
            public String call() throws Exception {
                return mXWalkViewInternal.getTitle();
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

    protected String getTitleOnUiThreadByContent(final XWalkViewInternal xWalkViewInternal) throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<String>() {
            @Override
            public String call() throws Exception {
                String title = xWalkViewInternal.getTitle();
                return title;
            }
        });
    }

    protected XWalkSettings getXWalkSettingsOnUiThreadByContent(
            final XWalkViewInternal xWalkViewInternal) throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<XWalkSettings>() {
            @Override
            public XWalkSettings call() throws Exception {
                return xWalkViewInternal.getSettings();
            }
        });
    }

    protected XWalkViewInternal createXWalkViewContainerOnMainSync(
            final Context context,
            final XWalkUIClientInternal uiClient,
            final XWalkResourceClientInternal resourceClient) throws Exception {
        final AtomicReference<XWalkViewInternal> xWalkViewContainer =
                new AtomicReference<XWalkViewInternal>();
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                xWalkViewContainer.set(new XWalkViewInternal(context, getActivity()));
                getActivity().addView(xWalkViewContainer.get());
                xWalkViewContainer.get().setUIClient(uiClient);
                xWalkViewContainer.get().setResourceClient(resourceClient);
            }
        });

        return xWalkViewContainer.get();
    }

    protected ViewPair createViewsOnMainSync(final TestHelperBridge helperBridge0,
                                             final TestHelperBridge helperBridge1,
                                             final XWalkUIClientInternal uiClient0,
                                             final XWalkUIClientInternal uiClient1,
                                             final XWalkResourceClientInternal resourceClient0,
                                             final XWalkResourceClientInternal resourceClient1,
                                             final Context context) throws Throwable {
        final XWalkViewInternal walkView0 = createXWalkViewContainerOnMainSync(context,
                uiClient0, resourceClient0);
        final XWalkViewInternal walkView1 = createXWalkViewContainerOnMainSync(context,
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

    protected XWalkViewInternal getXWalkView() {
        return mXWalkViewInternal;
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
                        mXWalkViewInternal.reload(mode);
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
                        mXWalkViewInternal.getNavigationHistory().navigate(
                            XWalkNavigationHistoryInternal.DirectionInternal.BACKWARD, 1);
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
                        mXWalkViewInternal.getNavigationHistory().navigate(
                            XWalkNavigationHistoryInternal.DirectionInternal.FORWARD, 1);
                    }
                });
            }
        });
    }

    protected void clearHistoryOnUiThread() throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkViewInternal.getNavigationHistory().clear();
            }
        });
    }

    protected boolean canGoBackOnUiThread() throws Throwable {
        return runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() {
                return mXWalkViewInternal.getNavigationHistory().canGoBack();
            }
        });
    }

    protected boolean canGoForwardOnUiThread() throws Throwable {
        return runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() {
                return mXWalkViewInternal.getNavigationHistory().canGoForward();
            }
        });
    }

    protected int historySizeOnUiThread() throws Throwable {
        return runTestOnUiThreadAndGetResult(new Callable<Integer>() {
            @Override
            public Integer call() {
                return mXWalkViewInternal.getNavigationHistory().size();
            }
        });
    }

    protected boolean hasItemAtOnUiThread(final int index) throws Throwable {
        return runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() {
                return mXWalkViewInternal.getNavigationHistory().hasItemAt(index);
            }
        });
    }

    protected XWalkNavigationItemInternal getItemAtOnUiThread(final int index) throws Throwable {
        return runTestOnUiThreadAndGetResult(new Callable<XWalkNavigationItemInternal>() {
            @Override
            public XWalkNavigationItemInternal call() {
                return mXWalkViewInternal.getNavigationHistory().getItemAt(index);
            }
        });
    }

    protected XWalkNavigationItemInternal getCurrentItemOnUiThread() throws Throwable {
        return runTestOnUiThreadAndGetResult(new Callable<XWalkNavigationItemInternal>() {
            @Override
            public XWalkNavigationItemInternal call() {
                return mXWalkViewInternal.getNavigationHistory().getCurrentItem();
            }
        });
    }

    protected String executeJavaScriptAndWaitForResult(final String code) throws Exception {
        final TestHelperBridge.OnEvaluateJavaScriptResultHelper helper =
                mTestHelperBridge.getOnEvaluateJavaScriptResultHelper();
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                helper.evaluateJavascript(mXWalkViewInternal, code);
            }
        });
        helper.waitUntilHasValue();
        Assert.assertTrue("Failed to retrieve JavaScript evaluation results.", helper.hasValue());
        return helper.getJsonResultAndClear();
    }

    protected ViewPair createViews() throws Throwable {
        TestHelperBridge helperBridge0 = new TestHelperBridge();
        TestHelperBridge helperBridge1 = new TestHelperBridge();
        TestXWalkUIClientInternalBase uiClient0 = new TestXWalkUIClientInternalBase(helperBridge0);
        TestXWalkUIClientInternalBase uiClient1 = new TestXWalkUIClientInternalBase(helperBridge1);
        TestXWalkResourceClientBase resourceClient0 =
                new TestXWalkResourceClientBase(helperBridge0);
        TestXWalkResourceClientBase resourceClient1 =
                new TestXWalkResourceClientBase(helperBridge1);
        ViewPair viewPair =
                createViewsOnMainSync(helperBridge0, helperBridge1, uiClient0, uiClient1,
                        resourceClient0, resourceClient1, getActivity());

        return viewPair;
    }

    protected String getUrlOnUiThread() throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<String>() {
            @Override
            public String call() throws Exception {
                return mXWalkViewInternal.getUrl();
            }
        });
    }

    protected void clearCacheOnUiThread(final boolean includeDiskFiles) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkViewInternal.clearCache(includeDiskFiles);
            }
        });
    }

    protected String getAPIVersionOnUiThread() throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<String>() {
            @Override
            public String call() throws Exception {
                return mXWalkViewInternal.getAPIVersion();
            }
        });
    }

    protected String getXWalkVersionOnUiThread() throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<String>() {
            @Override
            public String call() throws Exception {
                return mXWalkViewInternal.getXWalkVersion();
            }
        });
    }

    protected ContentViewCore getContentViewCore() throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<ContentViewCore>() {
            @Override
            public ContentViewCore call() throws Exception {
                return mXWalkViewInternal.getXWalkContentForTest();
            }
        });
    }
}
