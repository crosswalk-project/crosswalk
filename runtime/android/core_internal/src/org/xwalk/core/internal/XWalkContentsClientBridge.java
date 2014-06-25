// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.content.ContentResolver;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.Picture;
import android.net.Uri;
import android.net.http.SslCertificate;
import android.net.http.SslError;
import android.os.Message;
import android.provider.MediaStore;
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

    private XWalkViewInternal mXWalkView;
    private XWalkUIClientInternal mXWalkUIClient;
    private XWalkResourceClientInternal mXWalkResourceClient;
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
    private long mNativeContentsClientBridge;

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

    public XWalkContentsClientBridge(XWalkViewInternal xwView) {
        mXWalkView = xwView;

        mInterceptNavigationDelegate = new InterceptNavigationDelegateImpl(this);
    }

    public void setUIClient(XWalkUIClientInternal client) {
        // If it's null, use Crosswalk implementation.
        if (client != null) {
            mXWalkUIClient = client;
            return;
        }
        mXWalkUIClient = new XWalkUIClientInternal(mXWalkView);
    }

    public void setResourceClient(XWalkResourceClientInternal client) {
        // If it's null, use Crosswalk implementation.
        if (client != null) {
            mXWalkResourceClient = client;
            return;
        }
        mXWalkResourceClient = new XWalkResourceClientInternal(mXWalkView);
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

    private boolean isOwnerActivityRunning() {
        if (mXWalkView != null && mXWalkView.isOwnerActivityRunning()) {
            return true;
        }
        return false;
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
        if (isOwnerActivityRunning()) {
            mXWalkResourceClient.onProgressChanged(mXWalkView, progress);
        }
    }

    @Override
    public WebResourceResponse shouldInterceptRequest(String url) {
        if (isOwnerActivityRunning()) {
            return mXWalkResourceClient.shouldInterceptLoadRequest(mXWalkView, url);
        }
        return null;
    }

    @Override
    public void onResourceLoadStarted(String url) {
        if (isOwnerActivityRunning()) {
            mXWalkResourceClient.onLoadStarted(mXWalkView, url);
        }
    }

    @Override
    public void onResourceLoadFinished(String url) {
        if (isOwnerActivityRunning()) {
            mXWalkResourceClient.onLoadFinished(mXWalkView, url);
        }
    }

    @Override
    public void onLoadResource(String url) {
        if (mXWalkClient != null && isOwnerActivityRunning()) {
            mXWalkClient.onLoadResource(mXWalkView, url);
        }
    }

    @Override
    public boolean onConsoleMessage(ConsoleMessage consoleMessage) {
        return false;
    }

    @CalledByNative
    public void onReceivedHttpAuthRequest(XWalkHttpAuthHandler handler, String host, String realm) {
        if (mXWalkClient != null && isOwnerActivityRunning()) {
            mXWalkClient.onReceivedHttpAuthRequest(mXWalkView, handler, host, realm);
        }
    }

    @Override
    public void onReceivedSslError(ValueCallback<Boolean> callback, SslError error) {
        if (mXWalkClient != null && isOwnerActivityRunning()) {
            mXWalkClient.onReceivedSslError(mXWalkView, callback, error);
        }
    }

    @Override
    public void onReceivedLoginRequest(String realm, String account, String args) {
    }

    @Override
    public void onGeolocationPermissionsShowPrompt(String origin,
            XWalkGeolocationPermissions.Callback callback) {
        if (mXWalkWebChromeClient != null && isOwnerActivityRunning()) {
            mXWalkWebChromeClient.onGeolocationPermissionsShowPrompt(origin, callback);
        }
    }

    @Override
    public void onGeolocationPermissionsHidePrompt() {
        if (mXWalkWebChromeClient != null && isOwnerActivityRunning()) {
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
        if (mXWalkClient != null && isOwnerActivityRunning()) {
            mXWalkClient.onPageStarted(mXWalkView, url);
        }
    }

    @Override
    public void onPageFinished(String url) {
        if (!isOwnerActivityRunning()) return;
        if (mPageLoadListener != null) mPageLoadListener.onPageFinished(url);
        if (mXWalkClient != null) {
            mXWalkClient.onPageFinished(mXWalkView, url);
        }

        // This isn't the accurate point to notify a resource loading is finished,
        // but it's a workable way. We could enhance this by extending Content
        // API in future if we have the demand.
        onResourceLoadFinished(url);
    }

    @Override
    public void onReceivedError(int errorCode, String description, String failingUrl) {
        if (isOwnerActivityRunning()) {
            mXWalkResourceClient.onReceivedLoadError(mXWalkView, errorCode, description, failingUrl);
        }
    }

    @Override
    public void onRendererUnresponsive() {
        if (mXWalkClient != null && isOwnerActivityRunning()) {
            mXWalkClient.onRendererUnresponsive(mXWalkView);
        }
    }

    @Override
    public void onRendererResponsive() {
        if (mXWalkClient != null && isOwnerActivityRunning()) {
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
        if (isOwnerActivityRunning()) {
            mXWalkUIClient.onRequestFocus(mXWalkView);
        }
    }

    @Override
    public void onCloseWindow() {
        if (isOwnerActivityRunning()) {
            mXWalkUIClient.onJavascriptCloseWindow(mXWalkView);
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
        if (mXWalkWebChromeClient != null && isOwnerActivityRunning()) {
            mXWalkWebChromeClient.onShowCustomView(view, callback);
        }
    }

    @Override
    public void onHideCustomView() {
        if (mXWalkWebChromeClient != null && isOwnerActivityRunning()) {
            mXWalkWebChromeClient.onHideCustomView();
        }
    }

    @Override
    public void onScaleChangedScaled(float oldScale, float newScale) {
        if (isOwnerActivityRunning()) {
            mXWalkUIClient.onScaleChanged(mXWalkView, oldScale, newScale);
        }
    }

    @Override
    public void didFinishLoad(String url) {
    }

    @Override
    public void onTitleChanged(String title) {
        if (mXWalkWebChromeClient != null && isOwnerActivityRunning()) {
            mXWalkWebChromeClient.onReceivedTitle(mXWalkView, title);
        }
    }

    @Override
    public void onToggleFullscreen(boolean enterFullscreen) {
        if (isOwnerActivityRunning()) {
            mIsFullscreen = enterFullscreen;
            mXWalkUIClient.onFullscreenToggled(mXWalkView, enterFullscreen);
        }
    }

    @Override
    public boolean hasEnteredFullscreen() {
        return mIsFullscreen;
    }

    @Override
    public boolean shouldOverrideRunFileChooser(
            final int processId, final int renderId, final int modeFlags,
            String acceptTypes, boolean capture) {
        if (!isOwnerActivityRunning()) return false;
        abstract class UriCallback implements ValueCallback<Uri> {
            boolean syncNullReceived = false;
            boolean syncCallFinished = false;
            protected String resolveFileName(Uri uri, ContentResolver contentResolver) {
                if (contentResolver == null || uri == null) return "";
                Cursor cursor = null;
                try {
                    cursor = contentResolver.query(uri, null, null, null, null);

                    if (cursor != null && cursor.getCount() >= 1) {
                        cursor.moveToFirst();
                        int index = cursor.getColumnIndex(MediaStore.MediaColumns.DISPLAY_NAME);
                        if (index > -1) return cursor.getString(index);
                    }
                } catch (NullPointerException e) {
                    // Some android models don't handle the provider call correctly.
                    // see crbug.com/345393
                    return "";
                } finally {
                    if (cursor != null) cursor.close();
                }
                return "";
            }
        }
        UriCallback uploadFile = new UriCallback() {
            boolean completed = false;
            @Override
            public void onReceiveValue(Uri value) {
                if (completed) {
                    throw new IllegalStateException("Duplicate openFileChooser result");
                }
                completed = true;
                if (value == null && !syncCallFinished) {
                    syncNullReceived = true;
                    return;
                }
                if (value == null) {
                    nativeOnFilesNotSelected(mNativeContentsClientBridge,
                            processId, renderId, modeFlags);
                } else {
                    String result = "";
                    String displayName = null;
                    if (ContentResolver.SCHEME_FILE.equals(value.getScheme())) {
                        result = value.getSchemeSpecificPart();
                        displayName = value.getLastPathSegment();
                    } else if (ContentResolver.SCHEME_CONTENT.equals(value.getScheme())) {
                        result = value.toString();
                        displayName = resolveFileName(
                                value, mXWalkView.getActivity().getContentResolver());
                    } else {
                        result = value.getPath();
                        displayName = value.getLastPathSegment();
                    }
                    if (displayName == null || displayName.isEmpty()) displayName = result;
                    nativeOnFilesSelected(mNativeContentsClientBridge,
                            processId, renderId, modeFlags, result, displayName);
                }
            }
        };
        mXWalkUIClient.openFileChooser(
                mXWalkView, uploadFile, acceptTypes, Boolean.toString(capture));
        uploadFile.syncCallFinished = true;
        // File chooser requires user interaction, valid derives should handle it in async process.
        // If the ValueCallback receive a sync result with null value, it is considered the
        // file chooser is not overridden.
        return !uploadFile.syncNullReceived;
    }

    @Override
    public ContentVideoViewClient getContentVideoViewClient() {
        return new XWalkContentVideoViewClient(this, mXWalkView.getActivity(), mXWalkView);
    }

    // Used by the native peer to set/reset a weak ref to the native peer.
    @CalledByNative
    private void setNativeContentsClientBridge(long nativeContentsClientBridge) {
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
        if (isOwnerActivityRunning()) {
            XWalkJavascriptResultHandlerInternal result =
                    new XWalkJavascriptResultHandlerInternal(this, id);
            mXWalkUIClient.onJavascriptModalDialog(mXWalkView,
                    XWalkUIClientInternal.JavascriptMessageTypeInternal.JAVASCRIPT_ALERT,
                    url, message, "", result);
        }
    }

    @CalledByNative
    private void handleJsConfirm(String url, String message, int id) {
        if (isOwnerActivityRunning()) {
            XWalkJavascriptResultHandlerInternal result =
                    new XWalkJavascriptResultHandlerInternal(this, id);
            mXWalkUIClient.onJavascriptModalDialog(mXWalkView,
                    XWalkUIClientInternal.JavascriptMessageTypeInternal.JAVASCRIPT_CONFIRM,
                    url, message, "", result);
        }
    }

    @CalledByNative
    private void handleJsPrompt(String url, String message, String defaultValue, int id) {
        if (isOwnerActivityRunning()) {
            XWalkJavascriptResultHandlerInternal result =
                    new XWalkJavascriptResultHandlerInternal(this, id);
            mXWalkUIClient.onJavascriptModalDialog(mXWalkView,
                    XWalkUIClientInternal.JavascriptMessageTypeInternal.JAVASCRIPT_PROMPT,
                    url, message, defaultValue, result);
        }
    }

    @CalledByNative
    private void handleJsBeforeUnload(String url, String message, int id) {
        if (isOwnerActivityRunning()) {
            XWalkJavascriptResultHandlerInternal result =
                    new XWalkJavascriptResultHandlerInternal(this, id);
            mXWalkUIClient.onJavascriptModalDialog(mXWalkView,
                    XWalkUIClientInternal.JavascriptMessageTypeInternal.JAVASCRIPT_BEFOREUNLOAD,
                    url, message, "", result);
        }
    }

    @CalledByNative
    private void updateNotificationIcon(int notificationId, Bitmap icon) {
        mNotificationService.updateNotificationIcon(notificationId, icon);
    }

    @CalledByNative
    private void showNotification(String title, String message, String replaceId,
            int notificationId, long delegate) {
        // FIXME(wang16): use replaceId to replace exist notification. It happens when
        //                a notification with same name and tag fires.
        mNotificationService.showNotification(
                title, message, notificationId, delegate);
    }

    @CalledByNative
    private void cancelNotification(int notificationId, long delegate) {
        mNotificationService.cancelNotification(notificationId, delegate);
    }

    void confirmJsResult(int id, String prompt) {
        if (mNativeContentsClientBridge == 0) return;
        nativeConfirmJsResult(mNativeContentsClientBridge, id, prompt);
    }

    void cancelJsResult(int id) {
        if (mNativeContentsClientBridge == 0) return;
        nativeCancelJsResult(mNativeContentsClientBridge, id);
    }

    void exitFullscreen(long nativeWebContents) {
        if (mNativeContentsClientBridge == 0) return;
        nativeExitFullscreen(mNativeContentsClientBridge, nativeWebContents);
    }

    public void notificationDisplayed(long delegate) {
        if (mNativeContentsClientBridge == 0) return;
        nativeNotificationDisplayed(mNativeContentsClientBridge, delegate);
    }

    public void notificationError(long delegate) {
        if (mNativeContentsClientBridge == 0) return;
        nativeNotificationError(mNativeContentsClientBridge, delegate);
    }

    public void notificationClicked(int id, long delegate) {
        if (mNativeContentsClientBridge == 0) return;
        nativeNotificationClicked(mNativeContentsClientBridge, id, delegate);
    }

    public void notificationClosed(int id, boolean byUser, long delegate) {
        if (mNativeContentsClientBridge == 0) return;
        nativeNotificationClosed(mNativeContentsClientBridge, id, byUser, delegate);
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
    private native void nativeProceedSslError(long nativeXWalkContentsClientBridge,
            boolean proceed, int id);

    private native void nativeConfirmJsResult(long nativeXWalkContentsClientBridge, int id,
            String prompt);
    private native void nativeCancelJsResult(long nativeXWalkContentsClientBridge, int id);
    private native void nativeExitFullscreen(long nativeXWalkContentsClientBridge, long nativeWebContents);
    private native void nativeNotificationDisplayed(long nativeXWalkContentsClientBridge, long delegate);
    private native void nativeNotificationError(long nativeXWalkContentsClientBridge, long delegate);
    private native void nativeNotificationClicked(long nativeXWalkContentsClientBridge, int id, long delegate);
    private native void nativeNotificationClosed(long nativeXWalkContentsClientBridge, int id, boolean byUser, long delegate);
    private native void nativeOnFilesSelected(long nativeXWalkContentsClientBridge,
            int processId, int renderId, int mode_flags, String filepath, String displayName);
    private native void nativeOnFilesNotSelected(long nativeXWalkContentsClientBridge,
            int processId, int renderId, int mode_flags);
}
