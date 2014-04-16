// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.Intent;
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
import org.chromium.content.browser.DownloadInfo;

// Help bridge callback in XWalkContentsClient to XWalkViewClient and
// XWalkWebChromeClient; Also handle the JNI conmmunication logic.
@JNINamespace("xwalk")
class XWalkContentsClientBridge extends XWalkContentsClient
        implements ContentViewDownloadDelegate {

    private XWalkView mXWalkView;
    private XWalkUIClient mXWalkUIClient;
    private XWalkResourceClient mXWalkResourceClient;
    private XWalkClient mXWalkClient;
    private XWalkWebChromeClient mXWalkWebChromeClient;
    private Bitmap mFavicon;
    private DownloadListener mDownloadListener;
    private InterceptNavigationDelegate mInterceptNavigationDelegate;
    private PageLoadListener mPageLoadListener;
    private XWalkNavigationHandler mNavigationHandler;
    private XWalkNotificationService mNotificationService;
    private boolean mIsFullscreen = false;

    // The native peer of the object
    private int mNativeContentsClientBridge;

    private class InterceptNavigationDelegateImpl implements InterceptNavigationDelegate {
        private XWalkContentsClient mContentsClient;

        public InterceptNavigationDelegateImpl(XWalkContentsClient client) {
            mContentsClient = client;
        }

        public boolean shouldIgnoreNavigation(NavigationParams navigationParams) {
            final String url = navigationParams.url;
            boolean ignoreNavigation = shouldOverrideUrlLoading(url) ||
                     (mNavigationHandler != null &&
                      mNavigationHandler.handleNavigation(navigationParams));

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

    public void setUIClient(XWalkUIClient client) {
        // If it's null, use Crosswalk implementation.
        if (client != null) {
            mXWalkUIClient = client;
            return;
        }
        mXWalkUIClient = new XWalkUIClientImpl(mXWalkView.getContext(), mXWalkView);
    }

    public void setResourceClient(XWalkResourceClient client) {
        // If it's null, use Crosswalk implementation.
        if (client != null) {
            mXWalkResourceClient = client;
            return;
        }
        mXWalkResourceClient = new XWalkResourceClientImpl(mXWalkView.getContext(), mXWalkView);
    }


    public void setXWalkWebChromeClient(XWalkWebChromeClient client) {
        // If it's null, use Crosswalk implementation.
        if (client == null) return;
        client.setContentsClient(this);
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

    void registerPageLoadListener(PageLoadListener listener) {
        mPageLoadListener = listener;
    }

    public void setNotificationService(XWalkNotificationService service) {
        if (mNotificationService != null) mNotificationService.shutdown();
        mNotificationService = service;
        if (mNotificationService != null) mNotificationService.setBridge(this);
    }

    public boolean onNewIntent(Intent intent) {
        return mNotificationService.maybeHandleIntent(intent);
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
        mXWalkResourceClient.onProgressChanged(mXWalkView, progress);
    }

    @Override
    public WebResourceResponse shouldInterceptRequest(String url) {
        return mXWalkResourceClient.shouldInterceptLoadRequest(mXWalkView, url);
    }

    public void onResourceLoadStarted(String url) {
        mXWalkResourceClient.onLoadStarted(mXWalkView, url);
    }

    public void onResourceLoadFinished(String url) {
        // TODO(yongsheng): this method is never called. Where should it be hooked?
        mXWalkResourceClient.onLoadFinished(mXWalkView, url);
    }

    @Override
    public void onLoadResource(String url) {
        if (mXWalkClient != null && mXWalkView != null) {
            mXWalkClient.onLoadResource(mXWalkView, url);
        }
    }

    @Override
    public boolean onConsoleMessage(ConsoleMessage consoleMessage) {
        return false;
    }

    @CalledByNative
    public void onReceivedHttpAuthRequest(XWalkHttpAuthHandler handler, String host, String realm) {
        if (mXWalkClient != null && mXWalkView != null) {
            mXWalkClient.onReceivedHttpAuthRequest(mXWalkView, handler, host, realm);
        }
    }

    @Override
    public void onReceivedSslError(ValueCallback<Boolean> callback, SslError error) {
        if (mXWalkClient != null && mXWalkView != null) {
            mXWalkClient.onReceivedSslError(mXWalkView, callback, error);
        }
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
    public void onFindResultReceived(int activeMatchOrdinal, int numberOfMatches,
            boolean isDoneCounting) {
    }

    @Override
    public void onNewPicture(Picture picture) {
    }

    @Override
    public void onPageStarted(String url) {
        if (mXWalkClient != null && mXWalkView != null) {
            mXWalkClient.onPageStarted(mXWalkView, url);
        }
    }

    @Override
    public void onPageFinished(String url) {
        if (mPageLoadListener != null) mPageLoadListener.onPageFinished(url);
        if (mXWalkClient != null && mXWalkView != null) {
            mXWalkClient.onPageFinished(mXWalkView, url);
        }
    }

    @Override
    public void onReceivedError(int errorCode, String description, String failingUrl) {
        mXWalkResourceClient.onReceivedLoadError(mXWalkView, errorCode, description, failingUrl);
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
    public void onRequestFocus() {
        mXWalkUIClient.onRequestFocus(mXWalkView);
    }

    @Override
    public void onCloseWindow() {
        mXWalkUIClient.onJavascriptCloseWindow(mXWalkView);
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
        mXWalkUIClient.onScaleChanged(mXWalkView, oldScale, newScale);
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
    public void onToggleFullscreen(boolean enterFullscreen) {
        mIsFullscreen = enterFullscreen;
        mXWalkUIClient.onFullscreenToggled(mXWalkView, enterFullscreen);
    }

    @Override
    public boolean hasEnteredFullscreen() {
        return mIsFullscreen;
    }

    @Override
    public ContentVideoViewClient getContentVideoViewClient() {
        return new XWalkContentVideoViewClient(this, mXWalkView.getActivity(), mXWalkView);
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
        final SslCertificate cert = SslUtil.getCertificateFromDerBytes(derBytes);
        if (cert == null) {
            // if the certificate or the client is null, cancel the request
            return false;
        }
        final SslError sslError = SslUtil.sslErrorFromNetErrorCode(certError, cert, url);
        ValueCallback<Boolean> callback = new ValueCallback<Boolean>() {
            @Override
            public void onReceiveValue(Boolean value) {
                proceedSslError(value.booleanValue(), id);
            }
        };
        onReceivedSslError(callback, sslError);
        return true;
    }

    private void proceedSslError(boolean proceed, int id) {
        if (mNativeContentsClientBridge == 0) return;
        nativeProceedSslError(mNativeContentsClientBridge, proceed, id);
    }

    @CalledByNative
    private void handleJsAlert(String url, String message, int id) {
        XWalkJavascriptResultHandler result = new XWalkJavascriptResultHandler(this, id);
        mXWalkUIClient.onJavascriptModalDialog(mXWalkView,
                XWalkUIClient.JavascriptMessageType.JAVASCRIPT_ALERT, url, message, "", result);
    }

    @CalledByNative
    private void handleJsConfirm(String url, String message, int id) {
        XWalkJavascriptResultHandler result = new XWalkJavascriptResultHandler(this, id);
        mXWalkUIClient.onJavascriptModalDialog(mXWalkView,
                XWalkUIClient.JavascriptMessageType.JAVASCRIPT_CONFIRM, url, message, "", result);
    }

    @CalledByNative
    private void handleJsPrompt(String url, String message, String defaultValue, int id) {
        XWalkJavascriptResultHandler result = new XWalkJavascriptResultHandler(this, id);
        mXWalkUIClient.onJavascriptModalDialog(mXWalkView,
                XWalkUIClient.JavascriptMessageType.JAVASCRIPT_PROMPT, url, message, defaultValue,
                        result);
    }

    @CalledByNative
    private void handleJsBeforeUnload(String url, String message, int id) {
        XWalkJavascriptResultHandler result = new XWalkJavascriptResultHandler(this, id);
        mXWalkUIClient.onJavascriptModalDialog(mXWalkView,
                XWalkUIClient.JavascriptMessageType.JAVASCRIPT_BEFOREUNLOAD, url, message, "",
                        result);
    }

    // @CalledByNative
    // TODO(yongsheng): Native side should call file chooser.
    public void runFileChooser(final int processId, final int renderId, final int mode_flags,
            String acceptTypes, String title, String defaultFilename, boolean capture) {
        // TODO(yongsheng): Implement it.
        // mXWalkUIClient.openFileChooser(mXWalkView, ...);
        // See https://crosswalk-project.org/jira/browse/XWALK-1241.
    }

    @CalledByNative
    private void updateNotificationIcon(int notificationId, Bitmap icon) {
        mNotificationService.updateNotificationIcon(notificationId, icon);
    }

    @CalledByNative
    private void showNotification(String title, String message, String replaceId,
            int notificationId, int processId, int routeId) {
        // FIXME(wang16): use replaceId to replace exist notification. It happens when
        //                a notification with same name and tag fires.
        mNotificationService.showNotification(
                title, message, notificationId, processId, routeId);
    }

    @CalledByNative
    private void cancelNotification(int notificationId, int processId, int routeId) {
        mNotificationService.cancelNotification(notificationId, processId, routeId);
    }

    void confirmJsResult(int id, String prompt) {
        if (mNativeContentsClientBridge == 0) return;
        nativeConfirmJsResult(mNativeContentsClientBridge, id, prompt);
    }

    void cancelJsResult(int id) {
        if (mNativeContentsClientBridge == 0) return;
        nativeCancelJsResult(mNativeContentsClientBridge, id);
    }

    void exitFullscreen(int nativeWebContents) {
        if (mNativeContentsClientBridge == 0) return;
        nativeExitFullscreen(mNativeContentsClientBridge, nativeWebContents);
    }

    public void notificationDisplayed(int id, int processId, int routeId) {
        if (mNativeContentsClientBridge == 0) return;
        nativeNotificationDisplayed(mNativeContentsClientBridge, id, processId, routeId);
    }

    public void notificationError(int id, String error, int processId, int routeId) {
        if (mNativeContentsClientBridge == 0) return;
        nativeNotificationError(mNativeContentsClientBridge, id, error, processId, routeId);
    }

    public void notificationClicked(int id, int processId, int routeId) {
        if (mNativeContentsClientBridge == 0) return;
        nativeNotificationClicked(mNativeContentsClientBridge, id, processId, routeId);
    }

    public void notificationClosed(int id, boolean byUser, int processId, int routeId) {
        if (mNativeContentsClientBridge == 0) return;
        nativeNotificationClosed(mNativeContentsClientBridge, id, byUser, processId, routeId);
    }

    void setDownloadListener(DownloadListener listener) {
        mDownloadListener = listener;
    }

    // Implement ContentViewDownloadDelegate methods.
    public void requestHttpGetDownload(DownloadInfo downloadInfo) {
        if (mDownloadListener != null) {
            mDownloadListener.onDownloadStart(downloadInfo.getUrl(), downloadInfo.getUserAgent(),
            downloadInfo.getContentDisposition(), downloadInfo.getMimeType(), downloadInfo.getContentLength());
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
    private native void nativeExitFullscreen(int nativeXWalkContentsClientBridge, int nativeWebContents);
    private native void nativeNotificationDisplayed(int nativeXWalkContentsClientBridge, int id,
            int processId, int routeId);
    private native void nativeNotificationError(int nativeXWalkContentsClientBridge, int id,
            String error, int processId, int routeId);
    private native void nativeNotificationClicked(int nativeXWalkContentsClientBridge, int id,
            int processId, int routeId);
    private native void nativeNotificationClosed(int nativeXWalkContentsClientBridge, int id,
            boolean byUser, int processId, int routeId);
}
