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
import android.view.KeyEvent;
import android.view.View;
import android.webkit.ConsoleMessage;
import android.webkit.GeolocationPermissions;
import android.webkit.ValueCallback;
import android.webkit.WebResourceResponse;

import org.chromium.base.CalledByNative;
import org.chromium.base.JNINamespace;
import org.chromium.base.ThreadUtils;

// Help bridge callback in XwContentsClient to XwViewClient and
// WebChromeClient; Also handle the JNI conmmunication logic.
@JNINamespace("xwalk")
public class XwContentsClientBridge extends XwContentsClient {

    private XwView mXwView;
    private XwViewClient mXwViewClient;
    private XwWebChromeClient mXwWebChromeClient;
    private Bitmap mFavicon;

    // The native peer of the object
    private int mNativeContentsClientBridge;

    public XwContentsClientBridge(XwView xwView) {
        mXwView = xwView;
    }

    public void setXwWebChromeClient(XwWebChromeClient client) {
        mXwWebChromeClient = client;
    }

    public void setXwViewClient(XwViewClient client) {
        mXwViewClient = client;
    }

    // TODO(Xingnan): All the empty functions need to be implemented.
    @Override
    public boolean shouldOverrideUrlLoading(String url) {
        if (mXwViewClient != null && mXwView != null)
            return mXwViewClient.shouldOverrideUrlLoading(mXwView, url);
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
        if (mXwWebChromeClient != null && mXwView != null)
            mXwWebChromeClient.onProgressChanged(mXwView, progress);
    }

    @Override
    public WebResourceResponse shouldInterceptRequest(String url) {
        if (mXwViewClient != null && mXwView != null)
            return mXwViewClient.shouldInterceptRequest(mXwView, url);
        return null;
    }

    @Override
    public void onLoadResource(String url) {
        if (mXwViewClient != null && mXwView != null)
            mXwViewClient.onLoadResource(mXwView, url);
    }

    @Override
    public boolean onConsoleMessage(ConsoleMessage consoleMessage) {
        return false;
    }

    @Override
    public void onReceivedHttpAuthRequest(HttpAuthHandler handler, String host, String realm) {
        if (mXwViewClient != null && mXwView != null)
            mXwViewClient.onReceivedHttpAuthRequest(mXwView, handler, host, realm);
    }
    @Override
    public void onReceivedSslError(SslErrorHandler handler, SslError error) {
        if (mXwViewClient != null && mXwView != null)
            mXwViewClient.onReceivedSslError(mXwView, handler, error);
    }

    @Override
    public void onReceivedLoginRequest(String realm, String account, String args) {
    }

    @Override
    public void onGeolocationPermissionsShowPrompt(String origin,
            GeolocationPermissions.Callback callback) {
    }

    @Override
    public void onGeolocationPermissionsHidePrompt() {
    }

    @Override
    public void handleJsAlert(String url, String message, JsResult result) {
        if (mXwWebChromeClient != null && mXwView != null)
            mXwWebChromeClient.onJsAlert(mXwView, url, message, result);
    }

    @Override
    public void handleJsBeforeUnload(String url, String message, JsResult result) {
        if (mXwWebChromeClient != null && mXwView != null)
            mXwWebChromeClient.onJsBeforeUnload(mXwView, url, message, result);
    }

    @Override
    public void handleJsConfirm(String url, String message, JsResult result) {
        if (mXwWebChromeClient != null && mXwView != null)
            mXwWebChromeClient.onJsConfirm(mXwView, url, message, result);
    }

    @Override
    public void handleJsPrompt(
            String url, String message, String defaultValue, JsPromptResult result) {
        if (mXwWebChromeClient != null && mXwView != null)
            mXwWebChromeClient.onJsPrompt(mXwView, url, message, defaultValue, result);
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
        if (mXwViewClient != null && mXwView != null)
            mXwViewClient.onPageStarted(mXwView, url, mFavicon);
    }

    @Override
    public void onPageFinished(String url) {
        if (mXwViewClient != null && mXwView != null)
            mXwViewClient.onPageFinished(mXwView, url);
    }

    @Override
    public void onReceivedError(int errorCode, String description, String failingUrl) {
        if (mXwViewClient != null && mXwView != null)
            mXwViewClient.onReceivedError(mXwView, errorCode, description, failingUrl);
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
    }

    @Override
    public void onRequestFocus() {
    }

    @Override
    public void onReceivedTouchIconUrl(String url, boolean precomposed) {
        if (mXwWebChromeClient != null && mXwView != null)
            mXwWebChromeClient.onReceivedTouchIconUrl(mXwView, url, precomposed);
    }

    @Override
    public void onReceivedIcon(Bitmap bitmap) {
        if (mXwWebChromeClient != null && mXwView != null)
            mXwWebChromeClient.onReceivedIcon(mXwView, bitmap);
        mFavicon = bitmap;
    }

    @Override
    public void onShowCustomView(View view, XwWebChromeClient.CustomViewCallback callback) {
        if (mXwWebChromeClient != null)
            mXwWebChromeClient.onShowCustomView(view, callback);
    }

    @Override
    public void onHideCustomView() {
        if (mXwWebChromeClient != null)
            mXwWebChromeClient.onHideCustomView();
    }

    @Override
    public void onScaleChangedScaled(float oldScale, float newScale) {
    }

    @Override
    protected View getVideoLoadingProgressView() {
        if (mXwWebChromeClient != null)
            return mXwWebChromeClient.getVideoLoadingProgressView();
        return null;
    }

    @Override
    public Bitmap getDefaultVideoPoster() {
        return null;
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

    //--------------------------------------------------------------------------------------------
    //  Native methods
    //--------------------------------------------------------------------------------------------
    private native void nativeProceedSslError(int nativeXwContentsClientBridge,
            boolean proceed, int id);

    private native void nativeConfirmJsResult(int nativeXwContentsClientBridge, int id,
            String prompt);
    private native void nativeCancelJsResult(int nativeXwContentsClientBridge, int id);
}
