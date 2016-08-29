// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

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

import java.security.PrivateKey;
import java.util.ArrayList;
import java.util.HashMap;

import org.chromium.content.browser.ContentViewClient;
import org.chromium.content_public.browser.WebContents;
import org.chromium.content_public.browser.WebContentsObserver;
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
abstract class XWalkContentsClient extends ContentViewClient {

    private static final String TAG = "XWalkContentsClient";
    private final XWalkContentsClientCallbackHelper mCallbackHelper =
        new XWalkContentsClientCallbackHelper(this);

    private XWalkWebContentsObserver mWebContentsObserver;

    private double mDIPScale;


    public class XWalkWebContentsObserver extends WebContentsObserver {
        public XWalkWebContentsObserver(WebContents webContents) {
            super(webContents);
        }

        @Override
        public void didChangeThemeColor(int color) {
            boolean themecolor = XWalkPreferencesInternal.getValue(
                    XWalkPreferencesInternal.ENABLE_THEME_COLOR);
            if(themecolor) onDidChangeThemeColor(color);
        }

        @Override
        public void didStopLoading(String url) {
            mCallbackHelper.postOnPageFinished(url);
        }

        @Override
        public void didFailLoad(boolean isProvisionalLoad, boolean isMainFrame, int errorCode,
                String description, String failingUrl, boolean wasIgnoredByHandler) {
            if (errorCode == NetError.ERR_ABORTED || !isMainFrame) {
                // This error code is generated for the following reasons:
                // - XWalkViewInternal.stopLoading is called,
                // - the navigation is intercepted by the embedder via shouldOverrideNavigation.
                //
                // The XWalkViewInternal does not notify the embedder of these situations using this
                // error code with the XWalkClient.onReceivedError callback. What's more,
                // the XWalkViewInternal does not notify the embedder of sub-frame failures.
                return;
            }
            onReceivedError(ErrorCodeConversionHelper.convertErrorCode(errorCode),
                    description, failingUrl);
        }

        @Override
        public void didNavigateAnyFrame(String url, String baseUrl, boolean isReload) {
            doUpdateVisitedHistory(url, isReload);
        }

        @Override
        public void didFinishLoad(long frameId, String validatedUrl, boolean isMainFrame) {
            // Both didStopLoading and didFinishLoad will be called once a page is finished
            // to load, but didStopLoading will also be called when user clicks "X" button
            // on browser UI to stop loading page.
            //
            // So it is safe for Crosswalk to rely on didStopLoading to ensure onPageFinished
            // can be called.
        }

        @Override
        public void documentLoadedInFrame(long frameId, boolean isMainFrame) {
            onDocumentLoadedInFrame(frameId);
        }
    }

    @Override
    final public void onUpdateTitle(String title) {
        onTitleChanged(title);
    }

    @Override
    public boolean shouldOverrideKeyEvent(KeyEvent event) {
        return super.shouldOverrideKeyEvent(event);
    }

    void installWebContentsObserver(WebContents webContents) {
        if (mWebContentsObserver != null) {
            mWebContentsObserver.destroy();
        }
        mWebContentsObserver = new XWalkWebContentsObserver(webContents);
    }

    void setDIPScale(double dipScale) {
        mDIPScale = dipScale;
    }

    final XWalkContentsClientCallbackHelper getCallbackHelper() {
        return mCallbackHelper;
    }

    //--------------------------------------------------------------------------------------------
    //  XWalkViewInternal specific methods that map directly to XWalkViewClient/XWalkWebChromeClient
    //--------------------------------------------------------------------------------------------

    /**
     * Parameters for the {@link XWalkContentsClient#shouldInterceptRequest} method.
     */
    public static class WebResourceRequestInner {
        // Url of the request.
        public String url;
        // Is this for the main frame or a child iframe?
        public boolean isMainFrame;
        // Was a gesture associated with the request? Don't trust can easily be spoofed.
        public boolean hasUserGesture;
        // Method used (GET/POST/OPTIONS)
        public String method;
        // Headers that would have been sent to server.
        public HashMap<String, String> requestHeaders;
    }
    public abstract void getVisitedHistory(ValueCallback<String[]> callback);

    public abstract void doUpdateVisitedHistory(String url, boolean isReload);

    public abstract void onProgressChanged(int progress);

    public abstract XWalkWebResourceResponseInternal shouldInterceptRequest(
            WebResourceRequestInner request);

    public abstract void onResourceLoadStarted(String url);

    public abstract void onResourceLoadFinished(String url);

    public abstract void onLoadResource(String url);

    public abstract boolean shouldOverrideUrlLoading(String url);

    public abstract void onUnhandledKeyEvent(KeyEvent event);

    public abstract boolean onConsoleMessage(ConsoleMessage consoleMessage);

    public abstract void onReceivedHttpAuthRequest(XWalkHttpAuthHandlerInternal handler,
            String host, String realm);

    public abstract void onReceivedSslError(ValueCallback<Boolean> callback, SslError error);

    public abstract void onReceivedClientCertRequest(ClientCertRequestInternal handler);    

    public abstract void onReceivedResponseHeaders(WebResourceRequestInner request,
            XWalkWebResourceResponseInternal response);

    public abstract void onReceivedLoginRequest(String realm, String account, String args);

    public abstract void onFormResubmission(Message dontResend, Message resend);

    public abstract void onDownloadStart(String url, String userAgent, String contentDisposition,
            String mimeType, long contentLength);

    public abstract void onGeolocationPermissionsShowPrompt(String origin,
            XWalkGeolocationPermissions.Callback callback);

    public abstract void onGeolocationPermissionsHidePrompt();

    public final void onScaleChanged(float oldScaleFactor, float newScaleFactor) {
        onScaleChangedScaled((float)(oldScaleFactor * mDIPScale), (float)(newScaleFactor * mDIPScale));
    }

    public abstract void onScaleChangedScaled(float oldScale, float newScale);

    protected abstract boolean onCreateWindow(boolean isDialog, boolean isUserGesture);

    protected abstract void onCloseWindow();

    public abstract void onDidChangeThemeColor(int color);

    public abstract void onDocumentLoadedInFrame(long frameId);

    protected abstract void onRequestFocus();

    public abstract void onPageStarted(String url);

    public abstract void onPageFinished(String url);

    protected abstract void onStopLoading();

    public abstract void onReceivedError(int errorCode, String description, String failingUrl);

    public abstract void onRendererUnresponsive();

    public abstract void onRendererResponsive();

    public abstract void onTitleChanged(String title);

    public abstract void onToggleFullscreen(boolean enterFullscreen);

    public abstract boolean hasEnteredFullscreen();

    public abstract boolean shouldOverrideRunFileChooser(
            int processId, int renderId, int mode, String acceptTypes, boolean capture);

    // TODO (michaelbai): Remove this method once the same method remove from
    // XWalkContentsClientAdapter.
    public abstract void onShowCustomView(View view,
           int requestedOrientation, CustomViewCallbackInternal callback);

    // TODO (michaelbai): This method should be abstract, having empty body here
    // makes the merge to the Android easy.
    public void onShowCustomView(View view, CustomViewCallbackInternal callback) {
        onShowCustomView(view, ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED, callback);
    }

    public abstract void onHideCustomView();

    public abstract void didFinishLoad(String url);

    public abstract void provideClientCertificateResponse(int id, byte[][] certChain,
            PrivateKey privateKey);

    //--------------------------------------------------------------------------------------------
    //                              Other XWalkViewInternal-specific methods
    //--------------------------------------------------------------------------------------------
    //
    public abstract void onFindResultReceived(int activeMatchOrdinal, int numberOfMatches,
            boolean isDoneCounting);

    /**
     * Called whenever there is a new content picture available.
     * @param picture New picture.
     */
    public abstract void onNewPicture(Picture picture);

    public abstract boolean shouldCreateWebContents(String contentUrl);
}
