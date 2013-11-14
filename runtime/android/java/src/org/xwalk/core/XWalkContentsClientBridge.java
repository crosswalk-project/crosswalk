// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.graphics.Bitmap;
import android.graphics.Picture;
import android.net.http.SslCertificate;
import android.net.http.SslError;
import android.os.Message;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.webkit.ConsoleMessage;
import android.webkit.ValueCallback;
import android.webkit.WebResourceResponse;

import org.chromium.base.CalledByNative;
import org.chromium.base.JNINamespace;
import org.chromium.base.ThreadUtils;
import org.chromium.components.navigation_interception.InterceptNavigationDelegate;
import org.chromium.components.navigation_interception.NavigationParams;
import org.chromium.content.browser.ContentVideoViewClient;
import org.chromium.content.browser.ContentViewDownloadDelegate;

// Help bridge callback in XWalkContentsClient to XWalkViewClient and
// WebChromeClient; Also handle the JNI conmmunication logic.
@JNINamespace("xwalk")
public class XWalkContentsClientBridge extends XWalkContentsClient
        implements ContentViewDownloadDelegate {

    private XWalkView mXWalkView;
    private XWalkClient mXWalkClient;
    private XWalkWebChromeClient mXWalkWebChromeClient;
    private Bitmap mFavicon;
    private DownloadListener mDownloadListener;
    private InterceptNavigationDelegate mInterceptNavigationDelegate;
    private XWalkNavigationHandler mNavigationHandler;

    // The native peer of the object
    private int mNativeContentsClientBridge;

    private class InterceptNavigationDelegateImpl implements InterceptNavigationDelegate {
        private XWalkContentsClient mContentsClient;

        public InterceptNavigationDelegateImpl(XWalkContentsClient client) {
            mContentsClient = client;
        }

        public boolean shouldIgnoreNavigation(NavigationParams navigationParams) {
            final String url = navigationParams.url;
            boolean ignoreNavigation = false;

            if (mNavigationHandler != null) {
                ignoreNavigation = mNavigationHandler.handleNavigation(navigationParams);
            }
            if (!ignoreNavigation) ignoreNavigation = shouldOverrideUrlLoading(url);

            if (!ignoreNavigation) {
                // Post a message to UI thread to notify the page is starting to load.
                mContentsClient.getCallbackHelper().postOnPageStarted(url);
            }

            return ignoreNavigation;
        }
    }

    public XWalkContentsClientBridge(XWalkView xwView) {
        mXWalkView = xwView;

        mInterceptNavigationDelegate = new InterceptNavigationDelegateImpl(this);
    }

    public void setXWalkWebChromeClient(XWalkWebChromeClient client) {
        mXWalkWebChromeClient = client;
    }

    public XWalkWebChromeClient getXWalkWebChromeClient() {
        return mXWalkWebChromeClient;
    }

    public void setXWalkClient(XWalkClient client) {
        mXWalkClient = client;
    }

    public void setNavigationHandler(XWalkNavigationHandler handler) {
        mNavigationHandler = handler;
    }

    public InterceptNavigationDelegate getInterceptNavigationDelegate() {
        return mInterceptNavigationDelegate;
    }

    // TODO(Xingnan): All the empty functions need to be implemented.
    @Override
    public boolean shouldOverrideUrlLoading(String url) {
        if (mXWalkClient != null && mXWalkView != null)
            return mXWalkClient.shouldOverrideUrlLoading(mXWalkView, url);
        return false;
    }

    @Override
    public void onUnhandledKeyEvent(KeyEvent event) {
    }

    @Override
    public void getVisitedHistory(ValueCallback<String[]> callback) {
    }

    @Override
    public void doUpdateVisitedHistory(String url, boolean isReload) {
    }

    @Override
    public void onProgressChanged(int progress) {
        if (mXWalkWebChromeClient != null && mXWalkView != null)
            mXWalkWebChromeClient.onProgressChanged(mXWalkView, progress);
    }

    @Override
    public WebResourceResponse shouldInterceptRequest(String url) {
        if (mXWalkClient != null && mXWalkView != null)
            return mXWalkClient.shouldInterceptRequest(mXWalkView, url);
        return null;
    }

    @Override
    public void onLoadResource(String url) {
        if (mXWalkClient != null && mXWalkView != null)
            mXWalkClient.onLoadResource(mXWalkView, url);
    }

    @Override
    public boolean onConsoleMessage(ConsoleMessage consoleMessage) {
        return false;
    }

    @Override
    public void onReceivedHttpAuthRequest(HttpAuthHandler handler, String host, String realm) {
        if (mXWalkClient != null && mXWalkView != null)
            mXWalkClient.onReceivedHttpAuthRequest(mXWalkView, handler, host, realm);
    }
    @Override
    public void onReceivedSslError(SslErrorHandler handler, SslError error) {
        if (mXWalkClient != null && mXWalkView != null)
            mXWalkClient.onReceivedSslError(mXWalkView, handler, error);
    }

    @Override
    public void onReceivedLoginRequest(String realm, String account, String args) {
    }

    @Override
    public void onGeolocationPermissionsShowPrompt(String origin,
            XWalkGeolocationPermissions.Callback callback) {
        if (mXWalkWebChromeClient != null) {
            mXWalkWebChromeClient.onGeolocationPermissionsShowPrompt(origin, callback);
        }
    }

    @Override
    public void onGeolocationPermissionsHidePrompt() {
        if (mXWalkWebChromeClient != null) {
            mXWalkWebChromeClient.onGeolocationPermissionsHidePrompt();
        }
    }

    @Override
    public void handleJsAlert(String url, String message, JsResult result) {
        if (mXWalkWebChromeClient != null && mXWalkView != null) {
            mXWalkWebChromeClient.onJsAlert(mXWalkView, url, message, result);
        }
    }

    @Override
    public void handleJsBeforeUnload(String url, String message, JsResult result) {
        if (mXWalkWebChromeClient != null && mXWalkView != null) {
            mXWalkWebChromeClient.onJsBeforeUnload(mXWalkView, url, message, result);
        }
    }

    @Override
    public void handleJsConfirm(String url, String message, JsResult result) {
        if (mXWalkWebChromeClient != null && mXWalkView != null) {
            mXWalkWebChromeClient.onJsConfirm(mXWalkView, url, message, result);
        }
    }

    @Override
    public void handleJsPrompt(
            String url, String message, String defaultValue, JsPromptResult result) {
        if (mXWalkWebChromeClient != null && mXWalkView != null) {
            mXWalkWebChromeClient.onJsPrompt(mXWalkView, url, message, defaultValue, result);
        }
    }

    @Override
    public void onFindResultReceived(int activeMatchOrdinal, int numberOfMatches,
            boolean isDoneCounting) {
    }

    @Override
    public void onNewPicture(Picture picture) {
    }

    @Override
    public void onPageStarted(String url) {
        if (mXWalkClient != null && mXWalkView != null) {
            mXWalkClient.onPageStarted(mXWalkView, url, mFavicon);
        }
    }

    @Override
    public void onPageFinished(String url) {
        if (mXWalkClient != null && mXWalkView != null) {
            mXWalkClient.onPageFinished(mXWalkView, url);
        }
    }

    @Override
    public void onReceivedError(int errorCode, String description, String failingUrl) {
        if (mXWalkClient != null && mXWalkView != null) {
            mXWalkClient.onReceivedError(mXWalkView, errorCode, description, failingUrl);
        }
    }

    @Override
    public void onRendererUnresponsive() {
        if (mXWalkClient != null && mXWalkView != null) {
            mXWalkClient.onRendererUnresponsive(mXWalkView);
        }
    }

    @Override
    public void onRendererResponsive() {
        if (mXWalkClient != null && mXWalkView != null) {
            mXWalkClient.onRendererResponsive(mXWalkView);
        }
    }

    @Override
    public void onFormResubmission(Message dontResend, Message resend) {
        dontResend.sendToTarget();
    }

    @Override
    public void onDownloadStart(String url,
                                String userAgent,
                                String contentDisposition,
                                String mimeType,
                                long contentLength) {
    }

    @Override
    public boolean onCreateWindow(boolean isDialog, boolean isUserGesture) {
        return false;
    }

    @Override
    public void onCloseWindow() {
        if (mXWalkClient != null) {
            mXWalkClient.onCloseWindow(mXWalkView);
        }
    }

    @Override
    public void onRequestFocus() {
    }

    @Override
    public void onReceivedTouchIconUrl(String url, boolean precomposed) {
        if (mXWalkWebChromeClient != null && mXWalkView != null) {
            mXWalkWebChromeClient.onReceivedTouchIconUrl(mXWalkView, url, precomposed);
        }
    }

    @Override
    public void onReceivedIcon(Bitmap bitmap) {
        if (mXWalkWebChromeClient != null && mXWalkView != null) {
            mXWalkWebChromeClient.onReceivedIcon(mXWalkView, bitmap);
        }
        mFavicon = bitmap;
    }

    @Override
    public void onShowCustomView(View view, XWalkWebChromeClient.CustomViewCallback callback) {
        if (mXWalkWebChromeClient != null) {
            mXWalkWebChromeClient.onShowCustomView(view, callback);
        }
    }

    @Override
    public void onHideCustomView() {
        if (mXWalkWebChromeClient != null) {
            mXWalkWebChromeClient.onHideCustomView();
        }
    }

    @Override
    public void onScaleChangedScaled(float oldScale, float newScale) {
    }

    @Override
    protected View getVideoLoadingProgressView() {
        if (mXWalkWebChromeClient != null)
            return mXWalkWebChromeClient.getVideoLoadingProgressView();
        return null;
    }

    @Override
    public Bitmap getDefaultVideoPoster() {
        return null;
    }

    @Override
    public void didFinishLoad(String url) {
    }

    @Override
    public void onTitleChanged(String title) {
        if (mXWalkWebChromeClient != null && mXWalkView != null) {
            mXWalkWebChromeClient.onReceivedTitle(mXWalkView, title);
        }
    }

    @Override
    public ContentVideoViewClient getContentVideoViewClient() {
        return new XWalkContentVideoViewClient(this, mXWalkView.getActivity());
    }

    // Used by the native peer to set/reset a weak ref to the native peer.
    @CalledByNative
    private void setNativeContentsClientBridge(int nativeContentsClientBridge) {
        mNativeContentsClientBridge = nativeContentsClientBridge;
    }

    // If returns false, the request is immediately canceled, and any call to proceedSslError
    // has no effect. If returns true, the request should be canceled or proceeded using
    // proceedSslError().
    // Unlike the webview classic, we do not keep keep a database of certificates that
    // are allowed by the user, because this functionality is already handled via
    // ssl_policy in native layers.
    @CalledByNative
    private boolean allowCertificateError(int certError, byte[] derBytes, final String url,
            final int id) {
        // TODO(yongsheng): Implement this.
        return false;
    }

    private void proceedSslError(boolean proceed, int id) {
        if (mNativeContentsClientBridge == 0) return;
        nativeProceedSslError(mNativeContentsClientBridge, proceed, id);
    }

    @CalledByNative
    private void handleJsAlert(String url, String message, int id) {
        JsResultHandler handler = new JsResultHandler(this, id);
        handleJsAlert(url, message, handler);
    }

    @CalledByNative
    private void handleJsConfirm(String url, String message, int id) {
        JsResultHandler handler = new JsResultHandler(this, id);
        handleJsConfirm(url, message, handler);
    }

    @CalledByNative
    private void handleJsPrompt(String url, String message, String defaultValue, int id) {
        JsResultHandler handler = new JsResultHandler(this, id);
        handleJsPrompt(url, message, defaultValue, handler);
    }

    @CalledByNative
    private void handleJsBeforeUnload(String url, String message, int id) {
        JsResultHandler handler = new JsResultHandler(this, id);
        handleJsBeforeUnload(url, message, handler);
    }

    void confirmJsResult(int id, String prompt) {
        if (mNativeContentsClientBridge == 0) return;
        nativeConfirmJsResult(mNativeContentsClientBridge, id, prompt);
    }

    void cancelJsResult(int id) {
        if (mNativeContentsClientBridge == 0) return;
        nativeCancelJsResult(mNativeContentsClientBridge, id);
    }

    void setDownloadListener(DownloadListener listener) {
        mDownloadListener = listener;
    }

    // Implement ContentViewDownloadDelegate methods.
    public void requestHttpGetDownload(String url, String userAgent, String contentDisposition,
        String mimetype, String cookie, String referer, long contentLength) {
        if (mDownloadListener != null) {
            mDownloadListener.onDownloadStart(url, userAgent,
                    contentDisposition, mimetype, contentLength);
        }
    }

    public void onDownloadStarted(String filename, String mimeType) {
    }

    public void onDangerousDownload(String filename, int downloadId) {
    }

    //--------------------------------------------------------------------------------------------
    //  Native methods
    //--------------------------------------------------------------------------------------------
    private native void nativeProceedSslError(int nativeXWalkContentsClientBridge,
            boolean proceed, int id);

    private native void nativeConfirmJsResult(int nativeXWalkContentsClientBridge, int id,
            String prompt);
    private native void nativeCancelJsResult(int nativeXWalkContentsClientBridge, int id);
}
