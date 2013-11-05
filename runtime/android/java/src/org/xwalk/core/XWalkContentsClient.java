// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.pm.ActivityInfo;
import android.graphics.Bitmap;
import android.graphics.Picture;
import android.graphics.Rect;
import android.graphics.RectF;
import android.net.http.SslError;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.webkit.ConsoleMessage;
import android.webkit.ValueCallback;
import android.webkit.WebResourceResponse;

import org.chromium.content.browser.ContentViewClient;
import org.chromium.content.browser.ContentViewCore;
import org.chromium.content.browser.WebContentsObserverAndroid;
import org.chromium.net.NetError;

/**
 * Base-class that a XWalkViewContents embedder derives from to receive callbacks.
 * This extends ContentViewClient, as in many cases we want to pass-thru ContentViewCore
 * callbacks right to our embedder, and this setup facilities that.
 * For any other callbacks we need to make transformations of (e.g. adapt parameters
 * or perform filtering) we can provide final overrides for methods here, and then introduce
 * new abstract methods that the our own client must implement.
 * i.e.: all methods in this class should either be final, or abstract.
 */
public abstract class XWalkContentsClient extends ContentViewClient {

    private static final String TAG = "XWalkContentsClient";
    private final XWalkContentsClientCallbackHelper mCallbackHelper =
        new XWalkContentsClientCallbackHelper(this);

    private XWalkWebContentsObserver mWebContentsObserver;

    private double mDIPScale;

    public class XWalkWebContentsObserver extends WebContentsObserverAndroid {
        public XWalkWebContentsObserver(ContentViewCore contentViewCore) {
            super(contentViewCore);
        }

        @Override
        public void didStopLoading(String url) {
            onPageFinished(url);
        }

        @Override
        public void didFailLoad(boolean isProvisionalLoad,
                boolean isMainFrame, int errorCode, String description, String failingUrl) {
            // TODO(yongsheng): Implement this.
        }

        @Override
        public void didNavigateAnyFrame(String url, String baseUrl, boolean isReload) {
            doUpdateVisitedHistory(url, isReload);
        }

        @Override
        public void didFinishLoad(long frameId, String validatedUrl, boolean isMainFrame) {
            if (isMainFrame) {
                onPageFinished(validatedUrl);
            }
        }
    }

    void installWebContentsObserver(ContentViewCore contentViewCore) {
        if (mWebContentsObserver != null) {
            mWebContentsObserver.detachFromWebContents();
        }
        mWebContentsObserver = new XWalkWebContentsObserver(contentViewCore);
    }

    void setDIPScale(double dipScale) {
        mDIPScale = dipScale;
    }

    final XWalkContentsClientCallbackHelper getCallbackHelper() {
        return mCallbackHelper;
    }

    //--------------------------------------------------------------------------------------------
    //             XWalkView specific methods that map directly to XWalkViewClient / XWalkWebChromeClient
    //--------------------------------------------------------------------------------------------

    public abstract void getVisitedHistory(ValueCallback<String[]> callback);

    public abstract void doUpdateVisitedHistory(String url, boolean isReload);

    public abstract void onProgressChanged(int progress);

    public abstract WebResourceResponse shouldInterceptRequest(String url);

    public abstract void onLoadResource(String url);

    public abstract boolean shouldOverrideUrlLoading(String url);

    public abstract void onUnhandledKeyEvent(KeyEvent event);

    public abstract boolean onConsoleMessage(ConsoleMessage consoleMessage);

    public abstract void onReceivedHttpAuthRequest(HttpAuthHandler handler,
            String host, String realm);

    public abstract void onReceivedSslError(SslErrorHandler handler, SslError error);

    public abstract void onReceivedLoginRequest(String realm, String account, String args);

    public abstract void onFormResubmission(Message dontResend, Message resend);

    public abstract void onDownloadStart(String url, String userAgent, String contentDisposition,
            String mimeType, long contentLength);

    public abstract void onGeolocationPermissionsShowPrompt(String origin,
            XWalkGeolocationPermissions.Callback callback);

    public abstract void onGeolocationPermissionsHidePrompt();

    public final void onScaleChanged(float oldScale, float newScale) {
        onScaleChangedScaled((float)(oldScale * mDIPScale), (float)(newScale * mDIPScale));
    }

    public abstract void onScaleChangedScaled(float oldScale, float newScale);

    protected abstract void handleJsAlert(String url, String message, JsResult result);

    protected abstract void handleJsBeforeUnload(String url, String message,
            JsResult result);

    protected abstract void handleJsConfirm(String url, String message, JsResult result);

    protected abstract void handleJsPrompt(String url, String message, String defaultValue,
            JsPromptResult result);

    protected abstract boolean onCreateWindow(boolean isDialog, boolean isUserGesture);

    protected abstract void onCloseWindow();

    public abstract void onReceivedTouchIconUrl(String url, boolean precomposed);

    public abstract void onReceivedIcon(Bitmap bitmap);

    protected abstract void onRequestFocus();

    protected abstract View getVideoLoadingProgressView();

    public abstract void onPageStarted(String url);

    public abstract void onPageFinished(String url);

    public abstract void onReceivedError(int errorCode, String description, String failingUrl);

    public abstract void onRendererUnresponsive();

    public abstract void onRendererResponsive();

    final public void onUpdateTitle(String title) {
        onTitleChanged(title);
    }

    public abstract void onTitleChanged(String title);

    // TODO (michaelbai): Remove this method once the same method remove from
    // XWalkContentsClientAdapter.
    public void onShowCustomView(View view,
           int requestedOrientation, XWalkWebChromeClient.CustomViewCallback callback) {
    }

    // TODO (michaelbai): This method should be abstract, having empty body here
    // makes the merge to the Android easy.
    public void onShowCustomView(View view, XWalkWebChromeClient.CustomViewCallback callback) {
        onShowCustomView(view, ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED, callback);
    }

    public abstract void onHideCustomView();

    public abstract Bitmap getDefaultVideoPoster();

    public abstract void didFinishLoad(String url);

    //--------------------------------------------------------------------------------------------
    //                              Other XWalkView-specific methods
    //--------------------------------------------------------------------------------------------
    //
    public abstract void onFindResultReceived(int activeMatchOrdinal, int numberOfMatches,
            boolean isDoneCounting);

    /**
     * Called whenever there is a new content picture available.
     * @param picture New picture.
     */
    public abstract void onNewPicture(Picture picture);

    //--------------------------------------------------------------------------------------------
    //             Stuff that we ignore since it only makes sense for Chrome browser
    //--------------------------------------------------------------------------------------------
    //

    @Override
    final public boolean shouldOverrideScroll(float dx, float dy, float scrollX, float scrollY) {
        return false;
    }

    @Override
    final public void onContextualActionBarShown() {
    }

    @Override
    final public void onContextualActionBarHidden() {
    }
}
