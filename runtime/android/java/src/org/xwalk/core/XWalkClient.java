/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.xwalk.core;

import android.graphics.Bitmap;
import android.net.http.SslError;
import android.os.Message;
import android.view.KeyEvent;
import android.webkit.WebResourceResponse;

public class XWalkClient {

    /**
     * Give the host application a chance to take over the control when a new
     * url is about to be loaded in the current XWalkView. If XWalkClient is not
     * provided, by default XWalkView will ask Activity Manager to choose the
     * proper handler for the url. If XWalkClient is provided, return true
     * means the host application handles the url, while return false means the
     * current XWalkView handles the url.
     *
     * @param view The XWalkView that is initiating the callback.
     * @param url The url to be loaded.
     * @return True if the host application wants to leave the current XWalkView
     *         and handle the url itself, otherwise return false.
     */
    public boolean shouldOverrideUrlLoading(XWalkView view, String url) {
        return false;
    }

    /**
     * Notify the host application that a page has started loading. This method
     * is called once for each main frame load so a page with iframes or
     * framesets will call onPageStarted one time for the main frame. This also
     * means that onPageStarted will not be called when the contents of an
     * embedded frame changes, i.e. clicking a link whose target is an iframe.
     *
     * @param view The XWalkView that is initiating the callback.
     * @param url The url to be loaded.
     * @param favicon The favicon for this page if it already exists in the
     *            database.
     */
    public void onPageStarted(XWalkView view, String url, Bitmap favicon) {
    }

    /**
     * Notify the host application that a page has finished loading. This method
     * is called only for main frame. When onPageFinished() is called, the
     * rendering picture may not be updated yet. To get the notification for the
     * new Picture, use {@link XWalkView.PictureListener#onNewPicture}.
     *
     * @param view The XWalkView that is initiating the callback.
     * @param url The url of the page.
     */
    public void onPageFinished(XWalkView view, String url) {
    }

    /**
     * Notify the host application that a web page is closed by calling
     * window.close().
     */
    public void onCloseWindow(XWalkView view) {
    }

    /**
     * Notify the host application that the XWalkView will load the resource
     * specified by the given url.
     *
     * @param view The XWalkView that is initiating the callback.
     * @param url The url of the resource the XWalkView will load.
     */
    public void onLoadResource(XWalkView view, String url) {
    }

    /**
     * Notify the host application that the renderer of XWalkView is hung.
     *
     * @param view The XWalkView on which the render is hung.
     */
    public void onRendererUnresponsive(XWalkView view) {
    }

    /**
     * Notify the host application that the renderer of XWalkView is no longer hung.
     *
     * @param view The XWalkView which becomes responsive now.
     */
    public void onRendererResponsive(XWalkView view) {
    }

    /**
     * Notify the host application of a resource request and allow the
     * application to return the data.  If the return value is null, the XWalkView
     * will continue to load the resource as usual.  Otherwise, the return
     * response and data will be used.  NOTE: This method is called by the
     * network thread so clients should exercise caution when accessing private
     * data.
     *
     * @param view The {@link android.webkit.XWalkView} that is requesting the
     *             resource.
     * @param url The raw url of the resource.
     * @return A {@link android.webkit.WebResourceResponse} containing the
     *         response information or null if the XWalkView should load the
     *         resource itself.
     */
    public WebResourceResponse shouldInterceptRequest(XWalkView view,
            String url) {
        return null;
    }

    /**
     * Notify the host application that there have been an excessive number of
     * HTTP redirects. As the host application if it would like to continue
     * trying to load the resource. The default behavior is to send the cancel
     * message.
     *
     * @param view The XWalkView that is initiating the callback.
     * @param cancelMsg The message to send if the host wants to cancel
     * @param continueMsg The message to send if the host wants to continue
     * @deprecated This method is no longer called. When the XWalkView encounters
     *             a redirect loop, it will cancel the load.
     */
    @Deprecated
    public void onTooManyRedirects(XWalkView view, Message cancelMsg,
            Message continueMsg) {
        cancelMsg.sendToTarget();
    }

    // These ints must match up to the hidden values in EventHandler.
    /** Generic error */
    public static final int ERROR_UNKNOWN = -1;
    /** Server or proxy hostname lookup failed */
    public static final int ERROR_HOST_LOOKUP = -2;
    /** Unsupported authentication scheme (not basic or digest) */
    public static final int ERROR_UNSUPPORTED_AUTH_SCHEME = -3;
    /** User authentication failed on server */
    public static final int ERROR_AUTHENTICATION = -4;
    /** User authentication failed on proxy */
    public static final int ERROR_PROXY_AUTHENTICATION = -5;
    /** Failed to connect to the server */
    public static final int ERROR_CONNECT = -6;
    /** Failed to read or write to the server */
    public static final int ERROR_IO = -7;
    /** Connection timed out */
    public static final int ERROR_TIMEOUT = -8;
    /** Too many redirects */
    public static final int ERROR_REDIRECT_LOOP = -9;
    /** Unsupported URI scheme */
    public static final int ERROR_UNSUPPORTED_SCHEME = -10;
    /** Failed to perform SSL handshake */
    public static final int ERROR_FAILED_SSL_HANDSHAKE = -11;
    /** Malformed URL */
    public static final int ERROR_BAD_URL = -12;
    /** Generic file error */
    public static final int ERROR_FILE = -13;
    /** File not found */
    public static final int ERROR_FILE_NOT_FOUND = -14;
    /** Too many requests during this load */
    public static final int ERROR_TOO_MANY_REQUESTS = -15;

    /**
     * Report an error to the host application. These errors are unrecoverable
     * (i.e. the main resource is unavailable). The errorCode parameter
     * corresponds to one of the ERROR_* constants.
     * @param view The XWalkView that is initiating the callback.
     * @param errorCode The error code corresponding to an ERROR_* value.
     * @param description A String describing the error.
     * @param failingUrl The url that failed to load.
     */
    public void onReceivedError(XWalkView view, int errorCode,
            String description, String failingUrl) {
    }

    /**
     * As the host application if the browser should resend data as the
     * requested page was a result of a POST. The default is to not resend the
     * data.
     *
     * @param view The XWalkView that is initiating the callback.
     * @param dontResend The message to send if the browser should not resend
     * @param resend The message to send if the browser should resend data
     */
    public void onFormResubmission(XWalkView view, Message dontResend,
            Message resend) {
        dontResend.sendToTarget();
    }

    /**
     * Notify the host application to update its visited links database.
     *
     * @param view The XWalkView that is initiating the callback.
     * @param url The url being visited.
     * @param isReload True if this url is being reloaded.
     */
    public void doUpdateVisitedHistory(XWalkView view, String url,
            boolean isReload) {
    }

    /**
     * Notify the host application that an SSL error occurred while loading a
     * resource. The host application must call either handler.cancel() or
     * handler.proceed(). Note that the decision may be retained for use in
     * response to future SSL errors. The default behavior is to cancel the
     * load.
     *
     * @param view The XWalkView that is initiating the callback.
     * @param handler An SslErrorHandler object that will handle the user's
     *            response.
     * @param error The SSL error object.
     */
    public void onReceivedSslError(XWalkView view, SslErrorHandler handler,
            SslError error) {
        handler.cancel();
    }

    /**
     * Notify the host application that an SSL error occurred while loading a
     * resource, but the XWalkView chose to proceed anyway based on a
     * decision retained from a previous response to onReceivedSslError().
     * @hide
     */
    public void onProceededAfterSslError(XWalkView view, SslError error) {
    }

    /**
     * Notify the host application to handle a SSL client certificate
     * request (display the request to the user and ask whether to
     * proceed with a client certificate or not). The host application
     * has to call either handler.cancel() or handler.proceed() as the
     * connection is suspended and waiting for the response. The
     * default behavior is to cancel, returning no client certificate.
     *
     * @param view The XWalkView that is initiating the callback.
     * @param handler A ClientCertRequestHandler object that will
     *            handle the user's response.
     * @param host_and_port The host and port of the requesting server.
     *
     * @hide
     */
    // TODO: comment this method temporarily, will implemtent later when all
    //       dependencies are resovled.
    // public void onReceivedClientCertRequest(XWalkView view,
    //         ClientCertRequestHandler handler, String host_and_port) {
    //     handler.cancel();
    // }

    /**
     * Notify the host application to handle an authentication request. The
     * default behavior is to cancel the request.
     *
     * @param view The XWalkView that is initiating the callback.
     * @param handler The XWalkHttpAuthHandler that will handle the user's response.
     * @param host The host requiring authentication.
     * @param realm A description to help store user credentials for future
     *            visits.
     */
    public void onReceivedHttpAuthRequest(XWalkView view,
            XWalkHttpAuthHandler handler, String host, String realm) {
        handler.cancel();
    }

    /**
     * Give the host application a chance to handle the key event synchronously.
     * e.g. menu shortcut key events need to be filtered this way. If return
     * true, XWalkView will not handle the key event. If return false, XWalkView
     * will always handle the key event, so none of the super in the view chain
     * will see the key event. The default behavior returns false.
     *
     * @param view The XWalkView that is initiating the callback.
     * @param event The key event.
     * @return True if the host application wants to handle the key event
     *         itself, otherwise return false
     */
    public boolean shouldOverrideKeyEvent(XWalkView view, KeyEvent event) {
        return false;
    }

    /**
     * Notify the host application that a key was not handled by the XWalkView.
     * Except system keys, XWalkView always consumes the keys in the normal flow
     * or if shouldOverrideKeyEvent returns true. This is called asynchronously
     * from where the key is dispatched. It gives the host application a chance
     * to handle the unhandled key events.
     *
     * @param view The XWalkView that is initiating the callback.
     * @param event The key event.
     */
    public void onUnhandledKeyEvent(XWalkView view, KeyEvent event) {
        // TODO: Commment the below code for compile
        // ViewRootImpl root = view.getViewRootImpl();
        // if (root != null) {
        //     root.dispatchUnhandledKey(event);
        // }
    }

    /**
     * Notify the host application that the scale applied to the XWalkView has
     * changed.
     *
     * @param view he XWalkView that is initiating the callback.
     * @param oldScale The old scale factor
     * @param newScale The new scale factor
     */
    public void onScaleChanged(XWalkView view, float oldScale, float newScale) {
    }

    /**
     * Notify the host application that a request to automatically log in the
     * user has been processed.
     * @param view The XWalkView requesting the login.
     * @param realm The account realm used to look up accounts.
     * @param account An optional account. If not null, the account should be
     *                checked against accounts on the device. If it is a valid
     *                account, it should be used to log in the user.
     * @param args Authenticator specific arguments used to log in the user.
     */
    public void onReceivedLoginRequest(XWalkView view, String realm,
            String account, String args) {
    }
}
