// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.net.http.SslError;
import android.os.Build;
import android.text.InputType;
import android.util.Log;
import android.view.View;
import android.webkit.ValueCallback;
import android.webkit.WebResourceResponse;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Toast;

import java.io.InputStream;
import java.security.KeyStore.PrivateKeyEntry;
import java.security.Principal;
import java.util.ArrayList;
import java.util.Map;


/**
 * This class notifies the embedder resource events/callbacks.
 */
@XWalkAPI(createExternally = true)
public class XWalkResourceClientInternal {
    /**
     * Success
     * @since 1.0
     */
    @XWalkAPI
    public static final int ERROR_OK = 0;
    /**
     * Generic error
     * @since 1.0
     */
    @XWalkAPI
    public static final int ERROR_UNKNOWN = -1;
    /**
     * Server or proxy hostname lookup failed
     * @since 1.0
     */
    @XWalkAPI
    public static final int ERROR_HOST_LOOKUP = -2;
    /**
     * Unsupported authentication scheme (not basic or digest)
     * @since 1.0
     */
    @XWalkAPI
    public static final int ERROR_UNSUPPORTED_AUTH_SCHEME = -3;
    /**
     * User authentication failed on server
     * @since 1.0
     */
    @XWalkAPI
    public static final int ERROR_AUTHENTICATION = -4;
    /**
     * User authentication failed on proxy
     * @since 1.0
     */
    @XWalkAPI
    public static final int ERROR_PROXY_AUTHENTICATION = -5;
    /**
     * Failed to connect to the server
     * @since 1.0
     */
    @XWalkAPI
    public static final int ERROR_CONNECT = -6;
    /**
     * Failed to read or write to the server
     * @since 1.0
     */
    @XWalkAPI
    public static final int ERROR_IO = -7;
    /**
     * Connection timed out
     * @since 1.0
     */
    @XWalkAPI
    public static final int ERROR_TIMEOUT = -8;
    /**
     * Too many redirects
     * @since 1.0
     */
    @XWalkAPI
    public static final int ERROR_REDIRECT_LOOP = -9;
    /**
     * Unsupported URI scheme
     * @since 1.0
     */
    @XWalkAPI
    public static final int ERROR_UNSUPPORTED_SCHEME = -10;
    /**
     * Failed to perform SSL handshake
     * @since 1.0
     */
    @XWalkAPI
    public static final int ERROR_FAILED_SSL_HANDSHAKE = -11;
    /**
     * Malformed URL
     * @since 1.0
     */
    @XWalkAPI
    public static final int ERROR_BAD_URL = -12;
    /**
     * Generic file error
     * @since 1.0
     */
    @XWalkAPI
    public static final int ERROR_FILE = -13;
    /**
     * File not found
     * @since 1.0
     */
    @XWalkAPI
    public static final int ERROR_FILE_NOT_FOUND = -14;
    /**
     * Too many requests during this load
     * @since 1.0
     */
    @XWalkAPI
    public static final int ERROR_TOO_MANY_REQUESTS = -15;

    /**
     * Constructor.
     * @param view the owner XWalkViewInternal instance.
     * @since 1.0
     */
    @XWalkAPI
    public XWalkResourceClientInternal(XWalkViewInternal view) {
        // Keep the above parameter for future use.
    }

    /**
     * Notify the client that initial HTML document has been completely loaded and
     * parsed, without waiting for stylesheets, images, and subframes to finish loading.
     * This is similar to JavaScript DOMContentLoaded.
     * @param view the owner XWalkViewInternal instance.
     * @param frameId the loaded and parsed frame.
     * @since 5.0
     */
    @XWalkAPI
    public void onDocumentLoadedInFrame(XWalkViewInternal view, long frameId) {
    }

    /**
     * Notify the client that the XWalkViewInternal will load the resource specified
     * by the given url.
     * @param view the owner XWalkViewInternal instance.
     * @param url the url for the resource to be loaded.
     * @since 1.0
     */
    @XWalkAPI
    public void onLoadStarted(XWalkViewInternal view, String url) {
    }

    /**
     * Notify the client that the XWalkViewInternal completes to load the resource
     * specified by the given url.
     * @param view the owner XWalkViewInternal instance.
     * @param url the url for the resource done for loading.
     * @since 1.0
     */
    @XWalkAPI
    public void onLoadFinished(XWalkViewInternal view, String url) {
    }

    /**
     * Notify the client the progress info of loading a specific url.
     * @param view the owner XWalkViewInternal instance.
     * @param progressInPercent the loading process in percent.
     * @since 1.0
     */
    @XWalkAPI
    public void onProgressChanged(XWalkViewInternal view, int progressInPercent) {
    }

    /**
     * Notify the client of a resource request and allow the client to return
     * the data.  If the return value is null, the XWalkViewInternal
     * will continue to load the resource as usual.  Otherwise, the return
     * response and data will be used.  NOTE: This method is called by the
     * network thread so clients should exercise caution when accessing private
     * data.
     * @param view The owner XWalkViewInternal instance that is requesting the
     *             resource.
     * @param url The raw url of the resource.
     * @return A {@link android.webkit.WebResourceResponse} containing the
     *         response information or null if the XWalkViewInternal should load the
     *         resource itself.
     * @deprecated Use
     *        {@link #shouldInterceptLoadRequest(XWalkViewInternal, XWalkWebResourceRequestInternal)}
     *        instead.
     * @since 1.0
     */
    @XWalkAPI
    public WebResourceResponse shouldInterceptLoadRequest(XWalkViewInternal view, String url) {
        return null;
    }

    /**
     * Notify the client of a resource request and allow the client to return
     * the data.  If the return value is null, the XWalkViewInternal
     * will continue to load the resource as usual.  Otherwise, the return
     * response and data will be used.  NOTE: This method is called by the
     * network thread so clients should exercise caution when accessing private
     * data.
     * @param view The owner XWalkViewInternal instance that is requesting the
     *             resource.
     * @param request Object containing the details of the request..
     * @return A {@link org.xwalk.core.XWalkWebResourceResponse} containing the
     *         response information or null if the XWalkViewInternal should load the
     *         resource itself.
     * @since 6.0
     */
    @XWalkAPI
    public XWalkWebResourceResponseInternal shouldInterceptLoadRequest(XWalkViewInternal view,
            XWalkWebResourceRequestInternal request) {
        return null;
    }

    /**
     * Report an error to the client.
     * @param view the owner XWalkViewInternal instance.
     * @param errorCode the error id.
     * @param description A String describing the error.
     * @param failingUrl The url that failed to load.
     * @since 1.0
     */
    @XWalkAPI
    public void onReceivedLoadError(XWalkViewInternal view, int errorCode, String description,
            String failingUrl) {
        Toast.makeText(view.getContext(), description, Toast.LENGTH_SHORT).show();
    }

    /**
     * Give the host application a chance to take over the control when a new
     * url is about to be loaded in the current XWalkViewInternal. If XWalkClient is not
     * provided, by default XWalkViewInternal will ask Activity Manager to choose the
     * proper handler for the url. If XWalkClient is provided, return true
     * means the host application handles the url, while return false means the
     * current XWalkViewInternal handles the url.
     *
     * @param view The XWalkViewInternal that is initiating the callback.
     * @param url The url to be loaded.
     * @return True if the host application wants to leave the current XWalkViewInternal
     *         and handle the url itself, otherwise return false.
     *
     * @since 2.1
     */
    @XWalkAPI
    public boolean shouldOverrideUrlLoading(XWalkViewInternal view, String url) {
        return false;
    }

    /**
      * Notify the host application that an SSL error occurred while loading a
      * resource. The host application must call either callback.onReceiveValue(true)
      * or callback.onReceiveValue(false) . Note that the decision may be retained for
      * use in response to future SSL errors. The default behavior is to pop up a dialog
      * @param view the xwalkview that is initiating the callback
      * @param callback passing 'true' means accepting the ssl error and continue to load.
      *                 passing 'false' means forbidding to load the web page.
      * @param error the SSL error object
      * @since 4.0
      */
    @XWalkAPI
    public void onReceivedSslError(XWalkViewInternal view, ValueCallback<Boolean> callback,
            SslError error) {
        final ValueCallback<Boolean> valueCallback = callback;
        AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(view.getContext());
        dialogBuilder.setTitle(R.string.ssl_alert_title)
                .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        valueCallback.onReceiveValue(true);
                        dialog.dismiss();
                    }
                })
                .setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        valueCallback.onReceiveValue(false);
                        dialog.dismiss();
                    }
                }).setOnCancelListener(new DialogInterface.OnCancelListener() {
                    @Override
                    public void onCancel(DialogInterface dialog) {
                        valueCallback.onReceiveValue(false);
                    }
                });
        dialogBuilder.create().show();
    }

    /**
     * Notify the host application to handle a SSL client certificate request. The host application
     * is responsible for showing the UI if desired and providing the keys. There are three ways to
     * respond: proceed(), cancel() or ignore(). XWalkView remembers the response if proceed() or cancel()
     * is called and does not call onReceivedClientCertRequest() again for the same host and port pair.
     * XWalkView does not remember the response if ignore() is called.
     *
     * This method is called on the UI thread. During the callback, the connection is suspended.
     *
     * The default behavior is to cancel, returning no client certificate.
     *
     * @param view The XWalkView that is initiating the callback
     * @param handler An instance of a ClientCertRequestHandlerInternal
     *
     * @since 6.0
     */
    @XWalkAPI
    public void onReceivedClientCertRequest(XWalkViewInternal view,
            ClientCertRequestInternal handler) {
        handler.cancel();
    }

    /**
     * Notify the host application that an HTTP response has been received from the server while loading a resource.
     * This callback will be called for any resource (iframe, image, etc), not just for the main page.
     * Thus, it is recommended to perform minimum required work in this callback.
     * This method behaves similarly to the Android WebView's onReceivedHttpError if the HTTP response has a status code &gt;= 400.
     * If there are no errors, {@code response} contains the cookies set by the HTTP response.
     *
     * @param view The XWalkView that is initiating the callback
     * @param request The originating request
     * @param response The response information
     *
     * @since 6.0
     */
    @XWalkAPI
    public void onReceivedResponseHeaders(XWalkViewInternal view,
            XWalkWebResourceRequestInternal request,
            XWalkWebResourceResponseInternal response) {
    }

    /**
     * Notify the host application to update its visited links database.
     *
     * @param view The XWalkView that is initiating the callback.
     * @param url The url being visited.
     * @param isReload True if this url is being reloaded.
     *
     * @since 6.0
     */
    @XWalkAPI
    public void doUpdateVisitedHistory(XWalkViewInternal view, String url,
            boolean isReload) {
    }

    /**
     * Notify the host application to handle an authentication request. The
     * default behavior is to cancel the request.
     *
     * @param view The XWalkViewInternal that is initiating the callback.
     * @param handler The XWalkHttpAuthHandler that will handle the user's response.
     * @param host The host requiring authentication.
     * @param realm A description to help store user credentials for future
     *              visits.
     */
    @XWalkAPI
    public void onReceivedHttpAuthRequest(XWalkViewInternal view,
            XWalkHttpAuthHandlerInternal handler, String host, String realm) {
        if (view == null) return;

        final XWalkHttpAuthHandlerInternal haHandler = handler;
        Context context = view.getContext();
        LinearLayout layout = new LinearLayout(context);
        final EditText userNameEditText = new EditText(context);
        final EditText passwordEditText = new EditText(context);
        layout.setOrientation(LinearLayout.VERTICAL);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
            layout.setPaddingRelative(10, 0, 10, 20);
        } else {
            layout.setPadding(10, 0, 10, 20);
        }
        userNameEditText.setHint(R.string.http_auth_user_name);
        passwordEditText.setHint(R.string.http_auth_password);
        passwordEditText.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD);
        layout.addView(userNameEditText);
        layout.addView(passwordEditText);

        AlertDialog.Builder httpAuthDialog = new AlertDialog.Builder(view.getContext());
        httpAuthDialog.setTitle(R.string.http_auth_title)
                .setView(layout)
                .setCancelable(false)
                .setPositiveButton(R.string.http_auth_log_in,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int whichButton) {
                                String userName = userNameEditText.getText().toString();
                                String password = passwordEditText.getText().toString();
                                haHandler.proceed(userName, password);
                                dialog.dismiss();
                            }
                }).setNegativeButton(android.R.string.cancel, null)
                .setOnCancelListener(new DialogInterface.OnCancelListener() {
                    @Override
                    public void onCancel(DialogInterface dialog) {
                        haHandler.cancel();
                    }
                }).create().show();
    }

    /**
     * Construct an instance of XWalkWebResourceResponseInternal
     * for application usage.
     *
     * @param mimeType the resource response's MIME type, for example text/html
     * @param encoding the resource response's encoding
     * @param data the input stream that provides the resource response's data
     * @return XWalkWebResourceResponseInternal.
     * @since 6.0
     */
    @XWalkAPI
    public XWalkWebResourceResponseInternal createXWalkWebResourceResponse(
            String mimeType, String encoding, InputStream data) {
        return new XWalkWebResourceResponseInternal(mimeType, encoding, data);
    }

    /**
     * Construct an instance of XWalkWebResourceResponseInternal
     * for application usage.
     *
     * @param mimeType the resource response's MIME type, for example text/html
     * @param encoding the resource response's encoding
     * @param data the input stream that provides the resource response's data
     * @param statusCode the status code needs to be in the ranges [100, 299], [400, 599]
     * @param reasonPhrase the phrase describing the status code, for example "OK"
     * @param responseHeaders the resource response's headers represented as a mapping of header
     * @return XWalkWebResourceResponseInternal.
     * @since 6.0
     */
    @XWalkAPI
    public XWalkWebResourceResponseInternal createXWalkWebResourceResponse(
            String mimeType, String encoding, InputStream data, int statusCode,
            String reasonPhrase, Map<String, String> responseHeaders) {
        return new XWalkWebResourceResponseInternal(
            mimeType, encoding, data, statusCode, reasonPhrase, responseHeaders);
    }
}
