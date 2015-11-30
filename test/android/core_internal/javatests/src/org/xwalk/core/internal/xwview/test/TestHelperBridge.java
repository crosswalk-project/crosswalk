// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.xwview.test;

import android.util.Log;
import android.webkit.ValueCallback;
import android.webkit.WebResourceResponse;

import java.util.ArrayList;
import java.util.concurrent.Callable;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeoutException;
import java.util.List;

import org.chromium.content.browser.test.util.CallbackHelper;
import org.chromium.content.browser.test.util.Criteria;
import org.chromium.content.browser.test.util.TestCallbackHelperContainer.OnPageFinishedHelper;
import org.chromium.content.browser.test.util.TestCallbackHelperContainer.OnPageStartedHelper;
import org.chromium.content.browser.test.util.TestCallbackHelperContainer.OnReceivedErrorHelper;

import org.xwalk.core.internal.XWalkViewInternal;
import org.xwalk.core.internal.XWalkWebResourceResponseInternal;

class TestHelperBridge {

    // Two new helper classes for testing new APIs.
    public class ShouldInterceptLoadRequestHelper extends CallbackHelper {
        private List<String> mShouldInterceptRequestUrls = new ArrayList<String>();
        private ConcurrentHashMap<String, XWalkWebResourceResponseInternal> mReturnValuesByUrls
                = new ConcurrentHashMap<String, XWalkWebResourceResponseInternal>();
        // This is read from the IO thread, so needs to be marked volatile.
        private volatile XWalkWebResourceResponseInternal mResourceResponseReturnValue = null;
        private String mUrlToWaitFor;

        void setReturnValue(XWalkWebResourceResponseInternal value) {
            mResourceResponseReturnValue = value;
        }

        void setReturnValueForUrl(String url, XWalkWebResourceResponseInternal value) {
            mReturnValuesByUrls.put(url, value);
        }

        public void setUrlToWaitFor(String url) {
            mUrlToWaitFor = url;
        }

        public List<String> getUrls() {
            assert getCallCount() > 0;
            return mShouldInterceptRequestUrls;
        }

        public XWalkWebResourceResponseInternal getReturnValue(String url) {
            XWalkWebResourceResponseInternal value = mReturnValuesByUrls.get(url);
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

    public class OnLoadStartedHelper extends CallbackHelper {
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

    class OnEvaluateJavaScriptResultHelper extends CallbackHelper {
        private String mJsonResult;
        public void evaluateJavascript(XWalkViewInternal xWalkView, String code) {
            ValueCallback<String> callback =
                new ValueCallback<String>() {
                    @Override
                    public void onReceiveValue(String jsonResult) {
                        notifyCalled(jsonResult);
                    }
                };
            xWalkView.evaluateJavascript(code, callback);
            mJsonResult = null;
        }

        public boolean hasValue() {
            return mJsonResult != null;
        }

        public boolean waitUntilHasValue() throws InterruptedException, TimeoutException {
            waitUntilCriteria(getHasValueCriteria());
            return hasValue();
        }

        public String getJsonResultAndClear() {
            assert hasValue();
            String result = mJsonResult;
            mJsonResult = null;
            return result;
        }

        public Criteria getHasValueCriteria() {
            return new Criteria() {
                @Override
                public boolean isSatisfied() {
                    return hasValue();
                }
            };
        }

        public void notifyCalled(String jsonResult) {
            assert !hasValue();
            mJsonResult = jsonResult;
            notifyCalled();
        }
    }

    public class OnReceivedHttpAuthRequestHelper extends CallbackHelper {
        private String mHost;

        public String getHost() {
            assert getCallCount() > 0;
            return mHost;
        }

        public void notifyCalled(String host) {
            mHost = host;
            notifyCalled();
        }
    }

    private String mChangedTitle;
    private final OnPageStartedHelper mOnPageStartedHelper;
    private final OnPageFinishedHelper mOnPageFinishedHelper;
    private final OnReceivedErrorHelper mOnReceivedErrorHelper;

    private final OnEvaluateJavaScriptResultHelper mOnEvaluateJavaScriptResultHelper;

    private final OnTitleUpdatedHelper mOnTitleUpdatedHelper;
    private final ShouldInterceptLoadRequestHelper mShouldInterceptLoadRequestHelper;
    private final OnLoadStartedHelper mOnLoadStartedHelper;
    private final OnReceivedHttpAuthRequestHelper mOnReceivedHttpAuthRequestHelper;

    public TestHelperBridge() {
        mOnPageStartedHelper = new OnPageStartedHelper();
        mOnPageFinishedHelper = new OnPageFinishedHelper();
        mOnReceivedErrorHelper = new OnReceivedErrorHelper();
        mOnEvaluateJavaScriptResultHelper = new OnEvaluateJavaScriptResultHelper();
        mOnTitleUpdatedHelper = new OnTitleUpdatedHelper();
        mShouldInterceptLoadRequestHelper = new ShouldInterceptLoadRequestHelper();
        mOnLoadStartedHelper = new OnLoadStartedHelper();
        mOnReceivedHttpAuthRequestHelper = new OnReceivedHttpAuthRequestHelper();
    }

    public OnPageStartedHelper getOnPageStartedHelper() {
        return mOnPageStartedHelper;
    }

    public OnPageFinishedHelper getOnPageFinishedHelper() {
        return mOnPageFinishedHelper;
    }

    public OnReceivedErrorHelper getOnReceivedErrorHelper() {
        return mOnReceivedErrorHelper;
    }

    public OnEvaluateJavaScriptResultHelper getOnEvaluateJavaScriptResultHelper() {
        return mOnEvaluateJavaScriptResultHelper;
    }

    public OnTitleUpdatedHelper getOnTitleUpdatedHelper() {
        return mOnTitleUpdatedHelper;
    }

    public ShouldInterceptLoadRequestHelper getShouldInterceptLoadRequestHelper() {
        return mShouldInterceptLoadRequestHelper;
    }

    public OnLoadStartedHelper getOnLoadStartedHelper() {
        return mOnLoadStartedHelper;
    }

    public OnReceivedHttpAuthRequestHelper getOnReceivedHttpAuthRequestHelper() {
        return mOnReceivedHttpAuthRequestHelper;
    }

    public void onTitleChanged(String title) {
        mChangedTitle = title;
        mOnTitleUpdatedHelper.notifyCalled(title);
    }

    public String getChangedTitle() {
        return mChangedTitle;
    }

    public void onPageStarted(String url) {
        mOnPageStartedHelper.notifyCalled(url);
    }

    public void onPageFinished(String url) {
        mOnPageFinishedHelper.notifyCalled(url);
    }

    public void onReceivedLoadError(int errorCode, String description, String failingUrl) {
        mOnReceivedErrorHelper.notifyCalled(errorCode, description, failingUrl);
    }

    public XWalkWebResourceResponseInternal shouldInterceptLoadRequest(String url) {
        XWalkWebResourceResponseInternal response = mShouldInterceptLoadRequestHelper.getReturnValue(url);
        mShouldInterceptLoadRequestHelper.notifyCalled(url);
        return response;
    }

    public void onLoadStarted(String url) {
        mOnLoadStartedHelper.notifyCalled(url);
    }

    public void onReceivedHttpAuthRequest(String host) {
        mOnReceivedHttpAuthRequestHelper.notifyCalled(host);
    }
}
