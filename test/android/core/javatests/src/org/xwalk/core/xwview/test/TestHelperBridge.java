// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.net.Uri;
import android.util.Log;
import android.view.KeyEvent;
import android.webkit.ValueCallback;
import android.webkit.WebResourceResponse;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeoutException;

import org.chromium.content.browser.test.util.CallbackHelper;
import org.chromium.content.browser.test.util.Criteria;
import org.chromium.content.browser.test.util.TestCallbackHelperContainer.OnPageFinishedHelper;
import org.chromium.content.browser.test.util.TestCallbackHelperContainer.OnPageStartedHelper;
import org.chromium.content.browser.test.util.TestCallbackHelperContainer.OnReceivedErrorHelper;

import org.xwalk.core.XWalkUIClient.LoadStatus;
import org.xwalk.core.XWalkView;

class TestHelperBridge {

    // Two new helper classes for testing new APIs.
    public class ShouldInterceptLoadRequestHelper extends CallbackHelper {
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

    public class OnProgressChangedHelper extends CallbackHelper {
        private int mProgress;

        public int getProgress() {
            assert getCallCount() > 0;
            return mProgress;
        }

        public void notifyCalled(int progress) {
            mProgress = progress;
            notifyCalled();
        }
    }

    class OnEvaluateJavaScriptResultHelper extends CallbackHelper {
        private String mJsonResult;
        public void evaluateJavascript(XWalkView xWalkView, String code) {
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

    public class OnJavascriptCloseWindowHelper extends CallbackHelper {
        private boolean mCalled = false;

        public boolean getCalled() {
            assert getCallCount() > 0;
            return mCalled;
        }

        public void notifyCalled(boolean called) {
            mCalled = called;
            notifyCalled();
        }
    }

    public class ShouldOverrideUrlLoadingHelper extends CallbackHelper {
        private String mShouldOverrideUrlLoadingUrl;
        private String mPreviousShouldOverrideUrlLoadingUrl;
        private boolean mShouldOverrideUrlLoadingReturnValue = false;

        void setShouldOverrideUrlLoadingUrl(String url) {
            mShouldOverrideUrlLoadingUrl = url;
        }

        void setPreviousShouldOverrideUrlLoadingUrl(String url) {
            mPreviousShouldOverrideUrlLoadingUrl = url;
        }

        void setShouldOverrideUrlLoadingReturnValue(boolean value) {
            mShouldOverrideUrlLoadingReturnValue = value;
        }

        public String getShouldOverrideUrlLoadingUrl() {
            assert getCallCount() > 0;
            return mShouldOverrideUrlLoadingUrl;
        }

        public String getPreviousShouldOverrideUrlLoadingUrl() {
            assert getCallCount() > 1;
            return mPreviousShouldOverrideUrlLoadingUrl;
        }

        public boolean getShouldOverrideUrlLoadingReturnValue() {
            return mShouldOverrideUrlLoadingReturnValue;
        }

        public void notifyCalled(String url) {
            mPreviousShouldOverrideUrlLoadingUrl = mShouldOverrideUrlLoadingUrl;
            mShouldOverrideUrlLoadingUrl = url;
            notifyCalled();
        }
    }

    public class OnScaleChangedHelper extends CallbackHelper {
        private float mScale;

        public float getScale() {
            assert getCallCount() > 0;
            return mScale;
        }

        public void notifyCalled(float scale) {
            mScale = scale;
            notifyCalled();
        }
    }

    public class OnRequestFocusHelper extends CallbackHelper {
        private boolean mCalled = false;

        public boolean getCalled() {
            assert getCallCount() > 0;
            return mCalled;
        }

        public void notifyCalled(boolean called) {
            mCalled = called;
            notifyCalled();
        }
    }

    public class OnJavascriptModalDialogHelper extends CallbackHelper {
        private String mMessage;

        public String getMessage() {
            assert getCallCount() > 0;
            return mMessage;
        }

        public void notifyCalled(String message) {
            mMessage = message;
            notifyCalled();
        }
    }

    public class OpenFileChooserHelper extends CallbackHelper {
        private ValueCallback<Uri> mCallback;

        public ValueCallback<Uri> getCallback() {
            assert getCallCount() > 0;
            return mCallback;
        }

        public void notifyCalled(ValueCallback<Uri> callback) {
            mCallback = callback;
            notifyCalled();
        }
    }

    public class OnFullscreenToggledHelper extends CallbackHelper {
        private boolean mEnterFullscreen = false;

        public boolean getEnterFullscreen() {
            assert getCallCount() > 0;
            return mEnterFullscreen;
        }

        public void notifyCalled(boolean enterFullscreen) {
            mEnterFullscreen = enterFullscreen;
            notifyCalled();
        }
    }

    public class OverrideOrUnhandledKeyEventHelper extends CallbackHelper {
        private KeyEvent mEvent;

        public KeyEvent getKeyEvent() {
            assert getCallCount() > 0;
            return mEvent;
        }

        public void notifyCalled(KeyEvent event) {
            mEvent = event;
            notifyCalled();
        }
    }

    private String mChangedTitle;
    private LoadStatus mLoadStatus;
    private final OnPageStartedHelper mOnPageStartedHelper;
    private final OnPageFinishedHelper mOnPageFinishedHelper;
    private final OnReceivedErrorHelper mOnReceivedErrorHelper;

    private final OnEvaluateJavaScriptResultHelper mOnEvaluateJavaScriptResultHelper;

    private final OnTitleUpdatedHelper mOnTitleUpdatedHelper;
    private final ShouldInterceptLoadRequestHelper mShouldInterceptLoadRequestHelper;
    private final OnLoadStartedHelper mOnLoadStartedHelper;
    private final OnJavascriptCloseWindowHelper mOnJavascriptCloseWindowHelper;
    private final OnProgressChangedHelper mOnProgressChangedHelper;
    private final ShouldOverrideUrlLoadingHelper mShouldOverrideUrlLoadingHelper;
    private final OnScaleChangedHelper mOnScaleChangedHelper;
    private final OnRequestFocusHelper mOnRequestFocusHelper;
    private final OnJavascriptModalDialogHelper mOnJavascriptModalDialogHelper;
    private final OpenFileChooserHelper mOpenFileChooserHelper;
    private final OnFullscreenToggledHelper mOnFullscreenToggledHelper;
    private final OverrideOrUnhandledKeyEventHelper mOverrideOrUnhandledKeyEventHelper;

    public TestHelperBridge() {
        mOnPageStartedHelper = new OnPageStartedHelper();
        mOnPageFinishedHelper = new OnPageFinishedHelper();
        mOnReceivedErrorHelper = new OnReceivedErrorHelper();
        mOnEvaluateJavaScriptResultHelper = new OnEvaluateJavaScriptResultHelper();
        mOnTitleUpdatedHelper = new OnTitleUpdatedHelper();
        mShouldInterceptLoadRequestHelper = new ShouldInterceptLoadRequestHelper();
        mOnLoadStartedHelper = new OnLoadStartedHelper();
        mOnJavascriptCloseWindowHelper = new OnJavascriptCloseWindowHelper();
        mOnProgressChangedHelper = new OnProgressChangedHelper();
        mShouldOverrideUrlLoadingHelper = new ShouldOverrideUrlLoadingHelper();
        mOnScaleChangedHelper = new OnScaleChangedHelper();
        mOnRequestFocusHelper = new OnRequestFocusHelper();
        mOnJavascriptModalDialogHelper = new OnJavascriptModalDialogHelper();
        mOpenFileChooserHelper = new OpenFileChooserHelper();
        mOnFullscreenToggledHelper = new OnFullscreenToggledHelper();
        mOverrideOrUnhandledKeyEventHelper = new OverrideOrUnhandledKeyEventHelper();
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

    public OnJavascriptCloseWindowHelper getOnJavascriptCloseWindowHelper() {
        return mOnJavascriptCloseWindowHelper;
    }

    public OnProgressChangedHelper getOnProgressChangedHelper() {
        return mOnProgressChangedHelper;
    }

    public ShouldOverrideUrlLoadingHelper getShouldOverrideUrlLoadingHelper() {
        return mShouldOverrideUrlLoadingHelper;
    }

    public OnScaleChangedHelper getOnScaleChangedHelper() {
        return mOnScaleChangedHelper;
    }

    public OnRequestFocusHelper getOnRequestFocusHelper() {
        return mOnRequestFocusHelper;
    }

    public OnJavascriptModalDialogHelper getOnJavascriptModalDialogHelper() {
        return mOnJavascriptModalDialogHelper;
    }

    public OpenFileChooserHelper getOpenFileChooserHelper() {
        return mOpenFileChooserHelper;
    }

    public OnFullscreenToggledHelper getOnFullscreenToggledHelper() {
        return mOnFullscreenToggledHelper;
    }

    public OverrideOrUnhandledKeyEventHelper getOverrideOrUnhandledKeyEventHelper() {
        return mOverrideOrUnhandledKeyEventHelper;
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

    public void onPageFinished(String url, LoadStatus status) {
        mLoadStatus = status;
        mOnPageFinishedHelper.notifyCalled(url);
    }

    public LoadStatus getLoadStatus() {
        return mLoadStatus;
    }

    public void onReceivedLoadError(int errorCode, String description, String failingUrl) {
        mOnReceivedErrorHelper.notifyCalled(errorCode, description, failingUrl);
    }

    public WebResourceResponse shouldInterceptLoadRequest(String url) {
        WebResourceResponse response = mShouldInterceptLoadRequestHelper.getReturnValue(url);
        mShouldInterceptLoadRequestHelper.notifyCalled(url);
        return response;
    }

    public void onLoadStarted(String url) {
        mOnLoadStartedHelper.notifyCalled(url);
    }

    public void onJavascriptCloseWindow() {
        mOnJavascriptCloseWindowHelper.notifyCalled(true);
    }

    public void onProgressChanged(int progress) {
        mOnProgressChangedHelper.notifyCalled(progress);
    }

    public boolean shouldOverrideUrlLoading(String url) {
        boolean returnValue =
            mShouldOverrideUrlLoadingHelper.getShouldOverrideUrlLoadingReturnValue();
        mShouldOverrideUrlLoadingHelper.notifyCalled(url);
        return returnValue;
    }

    public void onScaleChanged(float scale) {
        mOnScaleChangedHelper.notifyCalled(scale);
    }

    public void onRequestFocus() {
        mOnRequestFocusHelper.notifyCalled(true);
    }

    public boolean onJavascriptModalDialog(String message) {
        mOnJavascriptModalDialogHelper.notifyCalled(message);
        return true;
    }

    public void openFileChooser(ValueCallback<Uri> uploadFile) {
        mOpenFileChooserHelper.notifyCalled(uploadFile);
    }

    public void onFullscreenToggled(boolean enterFullscreen) {
        mOnFullscreenToggledHelper.notifyCalled(enterFullscreen);
    }

    public boolean overrideOrUnhandledKeyEvent(KeyEvent event) {
        mOverrideOrUnhandledKeyEventHelper.notifyCalled(event);
        return true;
    }
}
