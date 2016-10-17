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
import android.os.Handler;
import android.provider.MediaStore;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.webkit.ConsoleMessage;
import android.webkit.ValueCallback;
import android.webkit.WebResourceResponse;
import android.widget.Toast;

import java.security.cert.X509Certificate;
import java.security.KeyStore.PrivateKeyEntry;
import java.security.Principal;
import java.security.PrivateKey;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.WeakHashMap;

import javax.security.auth.x500.X500Principal;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.base.ThreadUtils;
import org.chromium.components.navigation_interception.InterceptNavigationDelegate;
import org.chromium.components.navigation_interception.NavigationParams;
import org.chromium.content.browser.ContentVideoViewEmbedder;

import org.xwalk.core.internal.XWalkUIClientInternal.LoadStatusInternal;

// Help bridge callback in XWalkContentsClient to XWalkViewClient and
// XWalkWebChromeClient; Also handle the JNI conmmunication logic.
@JNINamespace("xwalk")
class XWalkContentsClientBridge extends XWalkContentsClient {

    private static final String TAG = XWalkContentsClientBridge.class.getName();
    private static final int NEW_XWALKVIEW_CREATED = 100;
    private static final int NEW_ICON_DOWNLOAD     = 101;

    private XWalkViewInternal mXWalkView;
    private XWalkUIClientInternal mXWalkUIClient;
    private XWalkResourceClientInternal mXWalkResourceClient;
    private XWalkClient mXWalkClient;
    private XWalkWebChromeClient mXWalkWebChromeClient;
    private Bitmap mFavicon;
    private XWalkDownloadListenerInternal mDownloadListener;
    private XWalkFindListenerInternal mFindListener;
    private InterceptNavigationDelegate mInterceptNavigationDelegate;
    private PageLoadListener mPageLoadListener;
    private XWalkNavigationHandler mNavigationHandler;
    private XWalkNotificationService mNotificationService;
    private Handler mUiThreadHandler;

    /** State recording variables */
    // For fullscreen state.
    private boolean mIsFullscreen = false;
    // For load state.
    private LoadStatusInternal mLoadStatus = LoadStatusInternal.FINISHED;
    private String mLoadingUrl = null;

    // The native peer of the object
    protected long mNativeContentsClientBridge;

    private float mPageScaleFactor;

    protected ClientCertLookupTable mLookupTable;

    private class InterceptNavigationDelegateImpl implements InterceptNavigationDelegate {
        private XWalkContentsClient mContentsClient;
        public InterceptNavigationDelegateImpl(XWalkContentsClient client) {
            mContentsClient = client;
        }

        public boolean shouldIgnoreNavigation(NavigationParams navigationParams) {
            final String url = navigationParams.url;
            boolean ignoreNavigation = mNavigationHandler != null &&
                     mNavigationHandler.handleNavigation(navigationParams);

            if (!ignoreNavigation) {
                // Check whether the fallback url is existed for scheme: intent://.
                final String fallbackUrl = mNavigationHandler.getFallbackUrl();
                if (fallbackUrl != null) {
                    mNavigationHandler.resetFallbackUrl();
                    mXWalkView.loadUrl(fallbackUrl);
                } else {
                    // Post a message to UI thread to notify the page is starting to load.
                    mContentsClient.getCallbackHelper().postOnPageStarted(url);
                }
            }

            return ignoreNavigation;
        }
    }

    public XWalkContentsClientBridge(XWalkViewInternal xwView) {
        mXWalkView = xwView;
        mLookupTable = new ClientCertLookupTable();
        mInterceptNavigationDelegate = new InterceptNavigationDelegateImpl(this);

        mUiThreadHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                switch(msg.what) {
                    case NEW_XWALKVIEW_CREATED:
                        XWalkViewInternal newXWalkView = (XWalkViewInternal) msg.obj;
                        if (newXWalkView == mXWalkView) {
                            throw new IllegalArgumentException("Parent XWalkView cannot host it's own popup window");
                        }

                        if (newXWalkView != null && newXWalkView.getNavigationHistory().size() != 0) {
                            throw new IllegalArgumentException("New WebView for popup window must not have been previously navigated.");
                        }

                        mXWalkView.completeWindowCreation(newXWalkView);
                        break;
                    case NEW_ICON_DOWNLOAD:
                        String url = (String) msg.obj;
                        nativeDownloadIcon(mNativeContentsClientBridge, url);
                        break;
                    default:
                        throw new IllegalStateException();
                }
            }
        };

    }

    public void setUIClient(XWalkUIClientInternal client) {
        // If it's null, use Crosswalk implementation.
        if (client == null) {
            mXWalkUIClient = new XWalkUIClientInternal(mXWalkView);
        } else {
            mXWalkUIClient = client;
        }
        mXWalkUIClient.setContentsClient(this);
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
        if (mXWalkResourceClient != null && mXWalkView != null) {
            return mXWalkResourceClient.shouldOverrideUrlLoading(mXWalkView, url);
        }
        return false;
    }

    @Override
    public boolean shouldOverrideKeyEvent(KeyEvent event) {
        boolean overridden = false;
        if (mXWalkUIClient != null && mXWalkView != null) {
            overridden = mXWalkUIClient.shouldOverrideKeyEvent(mXWalkView, event);
        }
        if (!overridden) {
            return super.shouldOverrideKeyEvent(event);
        }
        return overridden;
    }

    @Override
    public void onUnhandledKeyEvent(KeyEvent event) {
        if (mXWalkUIClient != null && mXWalkView != null) {
            mXWalkUIClient.onUnhandledKeyEvent(mXWalkView, event);
        }
    }

    @Override
    public void getVisitedHistory(ValueCallback<String[]> callback) {
    }

    @Override
    public void doUpdateVisitedHistory(String url, boolean isReload) {
        mXWalkResourceClient.doUpdateVisitedHistory(mXWalkView, url, isReload);
    }

    @Override
    public void onProgressChanged(int progress) {
        mXWalkResourceClient.onProgressChanged(mXWalkView, progress);
    }

    @Override
    public void onOverScrolled(int scrollX, int scrollY, boolean clampedX, boolean clampedY) {
        mXWalkView.onOverScrolledDelegate(scrollX, scrollY, clampedX, clampedY);
    }

    @Override
    public XWalkWebResourceResponseInternal shouldInterceptRequest(
            WebResourceRequestInner request) {
        //For compatibility with the old shouldInterceptLoadRequest.
        WebResourceResponse response =
             mXWalkResourceClient.shouldInterceptLoadRequest(mXWalkView, request.url);
        if (response == null) {
          XWalkWebResourceResponseInternal xwalkResponse =
                  mXWalkResourceClient.shouldInterceptLoadRequest(mXWalkView,
                          new XWalkWebResourceRequestHandlerInternal(request));
            if (xwalkResponse == null) return null;

            // XWalkWebResourceResponse should support null headers.
            Map<String, String> responseHeaders = xwalkResponse.getResponseHeaders();
            if (responseHeaders == null) responseHeaders = new HashMap<String, String>();

            //To Investigate: return xwalkResponse directly will fail, don't know why yet.
            return new XWalkWebResourceResponseInternal(
                    xwalkResponse.getMimeType(),
                    xwalkResponse.getEncoding(),
                    xwalkResponse.getData(),
                    xwalkResponse.getStatusCode(),
                    xwalkResponse.getReasonPhrase(),
                    responseHeaders);
        } else {
            return new XWalkWebResourceResponseInternal(
                    response.getMimeType(),
                    response.getEncoding(),
                    response.getData());
        }
    }

    @Override
    public void onDidChangeThemeColor(int color) {
        mXWalkUIClient.onDidChangeThemeColor(mXWalkView,color);
    }

    @Override
    public void onDocumentLoadedInFrame(long frameId) {
         mXWalkResourceClient.onDocumentLoadedInFrame(mXWalkView,frameId);
    }

    @Override
    public void onResourceLoadStarted(String url) {
        mXWalkResourceClient.onLoadStarted(mXWalkView, url);
    }

    @Override
    public void onResourceLoadFinished(String url) {
        mXWalkResourceClient.onLoadFinished(mXWalkView, url);
    }

    @Override
    public void onLoadResource(String url) {
        if (mXWalkClient != null) {
            mXWalkClient.onLoadResource(mXWalkView, url);
        }
    }

    @Override
    public boolean onConsoleMessage(ConsoleMessage consoleMessage) {
        if (mXWalkClient == null || mXWalkView == null) return false;
        XWalkUIClientInternal.ConsoleMessageType consoleMessageType =
            XWalkUIClientInternal.ConsoleMessageType.DEBUG;
        ConsoleMessage.MessageLevel messageLevel = consoleMessage.messageLevel();
        switch (messageLevel) {
            case TIP:
                consoleMessageType = XWalkUIClientInternal.ConsoleMessageType.INFO;
                break;
            case LOG:
                consoleMessageType = XWalkUIClientInternal.ConsoleMessageType.LOG;
                break;
            case WARNING:
                consoleMessageType = XWalkUIClientInternal.ConsoleMessageType.WARNING;
                break;
            case ERROR:
                consoleMessageType = XWalkUIClientInternal.ConsoleMessageType.ERROR;
                break;
            default:
                Log.w(TAG, "Unknown message level, defaulting to DEBUG");
                break;
        }
        return mXWalkUIClient.onConsoleMessage(mXWalkView, consoleMessage.message(),
                consoleMessage.lineNumber(), consoleMessage.sourceId(),
                consoleMessageType);
    }

    @CalledByNative
    public void onReceivedHttpAuthRequest(
            XWalkHttpAuthHandlerInternal handler, String host, String realm) {
        if (mXWalkResourceClient != null) {
            mXWalkResourceClient.onReceivedHttpAuthRequest(mXWalkView, handler, host, realm);
        }
    }

    @Override
    public void onReceivedSslError(ValueCallback<Boolean> callback, SslError error) {
        if (mXWalkResourceClient != null) {
            mXWalkResourceClient.onReceivedSslError(mXWalkView, callback, error);
        }
    }

    @Override
    public void onReceivedLoginRequest(String realm, String account, String args) {
    }

    @Override
    public void onReceivedClientCertRequest(ClientCertRequestInternal handler) {
        if (mXWalkResourceClient != null) {
            mXWalkResourceClient.onReceivedClientCertRequest(mXWalkView, handler);
        }
    }

    @Override
    public void onReceivedResponseHeaders(WebResourceRequestInner request,
            XWalkWebResourceResponseInternal response) {
        if (mXWalkResourceClient != null) {
            mXWalkResourceClient.onReceivedResponseHeaders(mXWalkView,
                    new XWalkWebResourceRequestHandlerInternal(request), response);
        }
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
        if (mFindListener == null) return;
        mFindListener.onFindResultReceived(activeMatchOrdinal, numberOfMatches, isDoneCounting);
    }

    @Override
    public void onNewPicture(Picture picture) {
    }

    @Override
    public void onPageStarted(String url) {
        if (mXWalkUIClient != null) {
            mLoadingUrl = url;
            mLoadStatus = LoadStatusInternal.FINISHED;
            mXWalkUIClient.onPageLoadStarted(mXWalkView, url);
        }
    }

    @Override
    public void onPageFinished(String url) {
        if (mPageLoadListener != null) mPageLoadListener.onPageFinished(url);
        if (mXWalkUIClient != null) {
            if (mLoadStatus == LoadStatusInternal.CANCELLED && mLoadingUrl != null) {
                mXWalkUIClient.onPageLoadStopped(mXWalkView, mLoadingUrl, mLoadStatus);
            } else {
                mXWalkUIClient.onPageLoadStopped(mXWalkView, url, mLoadStatus);
            }
            mLoadingUrl = null;
        }

        // This isn't the accurate point to notify a resource loading is finished,
        // but it's a workable way. We could enhance this by extending Content
        // API in future if we have the demand.
        onResourceLoadFinished(url);
    }

    @Override
    protected void onStopLoading() {
        mLoadStatus = LoadStatusInternal.CANCELLED;
    }

    @Override
    public void onReceivedError(int errorCode, String description, String failingUrl) {
        if (mLoadingUrl != null && mLoadingUrl.equals(failingUrl)) {
            mLoadStatus = LoadStatusInternal.FAILED;
        }
        mXWalkResourceClient.onReceivedLoadError(mXWalkView, errorCode, description, failingUrl);
    }

    @Override
    public void onRendererUnresponsive() {
        if (mXWalkClient != null) {
            mXWalkClient.onRendererUnresponsive(mXWalkView);
        }
    }

    @Override
    public void onRendererResponsive() {
        if (mXWalkClient != null) {
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
        if (mDownloadListener != null) {
            mDownloadListener.onDownloadStart(
                    url, userAgent, contentDisposition, mimeType, contentLength);
        }
    }

    @Override
    public boolean onCreateWindow(boolean isDialog, boolean isUserGesture) {
        if (isDialog) return false;

        XWalkUIClientInternal.InitiateByInternal initiator =
                XWalkUIClientInternal.InitiateByInternal.BY_JAVASCRIPT;
        if (isUserGesture) {
            initiator = XWalkUIClientInternal.InitiateByInternal.BY_USER_GESTURE;
        }

        ValueCallback<XWalkViewInternal> callback = new ValueCallback<XWalkViewInternal>() {
            @Override
            public void onReceiveValue(XWalkViewInternal newXWalkView) {
                Message m = mUiThreadHandler.obtainMessage(NEW_XWALKVIEW_CREATED, newXWalkView);
                m.sendToTarget();
            }
        };

        return mXWalkUIClient.onCreateWindowRequested(mXWalkView, initiator, callback);
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
    public void onShowCustomView(View view, CustomViewCallbackInternal callback) {
        if (mXWalkUIClient != null) {
            mXWalkUIClient.onShowCustomView(view, callback);
        }
    }

    @Override
    public void onShowCustomView(View view, int requestedOrientation,
            CustomViewCallbackInternal callback) {
        if (mXWalkUIClient != null) {
            mXWalkUIClient.onShowCustomView(view, requestedOrientation, callback);
        }
    }

    @Override
    public void onHideCustomView() {
        if (mXWalkUIClient != null) {
            mXWalkUIClient.onHideCustomView();
        }
    }

    @Override
    public void onScaleChangedScaled(float oldScale, float newScale) {
        mXWalkUIClient.onScaleChanged(mXWalkView, oldScale, newScale);
    }

    @Override
    public void didFinishLoad(String url) {
    }

    @Override
    public void onTitleChanged(String title) {
        if (mXWalkUIClient != null) {
            mXWalkUIClient.onReceivedTitle(mXWalkView, title);
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
    public boolean shouldCreateWebContents(String contentUrl) {
        return true;
    }

    @Override
    public boolean shouldOverrideRunFileChooser(
            final int processId, final int renderId, final int modeFlags,
            String acceptTypes, boolean capture) {
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
                if (value == null && !syncCallFinished) {
                    syncNullReceived = true;
                    return;
                }
                completed = true;
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
                                value, mXWalkView.getContext().getContentResolver());
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
        return !uploadFile.syncNullReceived;
    }

    @Override
    public ContentVideoViewEmbedder getContentVideoViewEmbedder() {
        return new XWalkContentVideoViewClient(this, mXWalkView);
    }

    public void provideClientCertificateResponse(int id, byte[][] certChain,
            PrivateKey privateKey) {
        nativeProvideClientCertificateResponse(mNativeContentsClientBridge, id, certChain, privateKey);
    }

    public Bitmap getFavicon() {
        return mFavicon;
    }

    // Used by the native peer to set/reset a weak ref to the native peer.
    @CalledByNative
    private void setNativeContentsClientBridge(long nativeContentsClientBridge) {
        mNativeContentsClientBridge = nativeContentsClientBridge;
    }

    // If returns false, the request is immediately canceled, and any call to proceedSslError
    // has no effect. If returns true, the request should be canceled or proceeded using
    // proceedSslError().
    // Unlike the webview classic, we do not keep a database of certificates that
    // are allowed by the user, because this functionality is already handled via
    // ssl_policy in native layers.
    @CalledByNative
    private boolean allowCertificateError(int certError, byte[] derBytes, final String url,
            final int id) {
        boolean shouldDeny = SslUtil.shouldDenyRequest(certError);
        if (shouldDeny) {
            Toast.makeText(mXWalkView.getContext(), R.string.ssl_error_deny_request,
                    Toast.LENGTH_SHORT).show();
            return false;
        }
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

    @CalledByNative
    private void selectClientCertificate(final int id, final String[] keyTypes,
            byte[][] encodedPrincipals, final String host, final int port) {
        if (mXWalkResourceClient != null) {
            assert mNativeContentsClientBridge != 0;

            ClientCertLookupTable.Cert cert = mLookupTable.getCertData(host, port);

            if (mLookupTable.isDenied(host, port)) {
                nativeProvideClientCertificateResponse(mNativeContentsClientBridge, id, null, null);

                return;
            }

            if (cert != null) {
                nativeProvideClientCertificateResponse(mNativeContentsClientBridge, id,
                        cert.mCertChain, cert.mPrivateKey);

                return;
            }

            // Build the list of principals from encoded versions.
            Principal[] principals = null;

            if (encodedPrincipals.length > 0) {
                principals = new X500Principal[encodedPrincipals.length];

                for (int n = 0; n < encodedPrincipals.length; n++) {
                    try {
                        principals[n] = new X500Principal(encodedPrincipals[n]);
                    } catch (IllegalArgumentException e) {
                        Log.w(TAG, "Exception while decoding issuers list: " + e);
                        nativeProvideClientCertificateResponse(mNativeContentsClientBridge, id,
                                null, null);

                        return;
                    }
                }
            }

            final ClientCertRequestInternal handler = new ClientCertRequestHandlerInternal(
                    this, id, keyTypes, principals, host, port);
            this.onReceivedClientCertRequest(handler);
        }
    }

    public void clearClientCertPreferences(Runnable callback) {
        mLookupTable.clear();

        if (mNativeContentsClientBridge != 0) {
            nativeClearClientCertPreferences(mNativeContentsClientBridge, callback);
        } else if (callback != null) {
            callback.run();
        }
    }

    @CalledByNative
    private void clientCertificatesCleared(Runnable callback) {
        if (callback == null) return;
        callback.run();
    }

    private void proceedSslError(boolean proceed, int id) {
        if (mNativeContentsClientBridge == 0) return;
        nativeProceedSslError(mNativeContentsClientBridge, proceed, id);
    }

    @CalledByNative
    private void handleJsAlert(String url, String message, int id) {
        XWalkJavascriptResultHandlerInternal result =
                new XWalkJavascriptResultHandlerInternal(this, id);
        mXWalkUIClient.onJavascriptModalDialog(mXWalkView,
                XWalkUIClientInternal.JavascriptMessageTypeInternal.JAVASCRIPT_ALERT,
                url, message, "", result);
    }

    @CalledByNative
    private void handleJsConfirm(String url, String message, int id) {
        XWalkJavascriptResultHandlerInternal result =
                new XWalkJavascriptResultHandlerInternal(this, id);
        mXWalkUIClient.onJavascriptModalDialog(mXWalkView,
                XWalkUIClientInternal.JavascriptMessageTypeInternal.JAVASCRIPT_CONFIRM,
                url, message, "", result);
    }

    @CalledByNative
    private void handleJsPrompt(String url, String message, String defaultValue, int id) {
        XWalkJavascriptResultHandlerInternal result =
                new XWalkJavascriptResultHandlerInternal(this, id);
        mXWalkUIClient.onJavascriptModalDialog(mXWalkView,
                XWalkUIClientInternal.JavascriptMessageTypeInternal.JAVASCRIPT_PROMPT,
                url, message, defaultValue, result);
    }

    @CalledByNative
    private void handleJsBeforeUnload(String url, String message, int id) {
        XWalkJavascriptResultHandlerInternal result =
                new XWalkJavascriptResultHandlerInternal(this, id);
        mXWalkUIClient.onJavascriptModalDialog(mXWalkView,
                XWalkUIClientInternal.JavascriptMessageTypeInternal.JAVASCRIPT_BEFOREUNLOAD,
                url, message, "", result);
    }

    @CalledByNative
    private boolean shouldOverrideUrlLoading(
            String url, boolean hasUserGesture, boolean isRedirect, boolean isMainFrame) {
         return shouldOverrideUrlLoading(url);
    }

    @CalledByNative
    private void showNotification(String title, String message, String replaceId,
            Bitmap icon, int notificationId) {
        mNotificationService.showNotification(
                title, message, replaceId, icon, notificationId);
    }

    @CalledByNative
    private void cancelNotification(int notificationId) {
        mNotificationService.cancelNotification(notificationId);
    }

    void confirmJsResult(int id, String prompt) {
        if (mNativeContentsClientBridge == 0) return;
        nativeConfirmJsResult(mNativeContentsClientBridge, id, prompt);
    }

    void cancelJsResult(int id) {
        if (mNativeContentsClientBridge == 0) return;
        nativeCancelJsResult(mNativeContentsClientBridge, id);
    }

    public void notificationDisplayed(int id) {
        if (mNativeContentsClientBridge == 0) return;
        nativeNotificationDisplayed(mNativeContentsClientBridge, id);
    }

    public void notificationClicked(int id) {
        if (mNativeContentsClientBridge == 0) return;
        nativeNotificationClicked(mNativeContentsClientBridge, id);
    }

    public void notificationClosed(int id, boolean byUser) {
        if (mNativeContentsClientBridge == 0) return;
        nativeNotificationClosed(mNativeContentsClientBridge, id, byUser);
    }

    void setDownloadListener(XWalkDownloadListenerInternal listener) {
        mDownloadListener = listener;
    }

    void setFindListener(XWalkFindListenerInternal listener) {
        mFindListener = listener;
    }

    @CalledByNative
    public void onWebLayoutPageScaleFactorChanged(float pageScaleFactor) {
        if (mPageScaleFactor == pageScaleFactor) return;

        float oldPageScaleFactor = mPageScaleFactor;
        mPageScaleFactor = pageScaleFactor;
        onScaleChanged(oldPageScaleFactor, mPageScaleFactor);
    }

    @CalledByNative
    public void onIconAvailable(String url) {
        Message m = mUiThreadHandler.obtainMessage(NEW_ICON_DOWNLOAD, url);
        mXWalkUIClient.onIconAvailable(mXWalkView, url, m);
    }

    @CalledByNative
    public void onReceivedIcon(String url, Bitmap icon) {
        mXWalkUIClient.onReceivedIcon(mXWalkView, url, icon);
        mFavicon = icon;
    }

    //--------------------------------------------------------------------------------------------
    //  Native methods
    //--------------------------------------------------------------------------------------------
    private native void nativeProceedSslError(long nativeXWalkContentsClientBridge,
            boolean proceed, int id);

    private native void nativeConfirmJsResult(long nativeXWalkContentsClientBridge, int id,
            String prompt);
    private native void nativeCancelJsResult(long nativeXWalkContentsClientBridge, int id);
    private native void nativeNotificationDisplayed(long nativeXWalkContentsClientBridge, int id);
    private native void nativeNotificationClicked(long nativeXWalkContentsClientBridge, int id);
    private native void nativeNotificationClosed(long nativeXWalkContentsClientBridge, int id, boolean byUser);
    private native void nativeOnFilesSelected(long nativeXWalkContentsClientBridge,
            int processId, int renderId, int mode_flags, String filepath, String displayName);
    private native void nativeOnFilesNotSelected(long nativeXWalkContentsClientBridge,
            int processId, int renderId, int mode_flags);
    private native void nativeDownloadIcon(long nativeXWalkContentsClientBridge, String url);
    private native void nativeProvideClientCertificateResponse(
            long nativeXWalkContentsClientBridge, int id, byte[][] certChain, PrivateKey privateKey);
    private native void nativeClearClientCertPreferences(
            long nativeXWalkContentsClientBridge, Runnable callback);
}
