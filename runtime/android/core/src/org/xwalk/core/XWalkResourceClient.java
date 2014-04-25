// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.view.View;
import android.webkit.WebResourceResponse;

/**
 * This class notifies the embedder resource events/callbacks.
 */
public class XWalkResourceClient {
    /** Success */
    public static final int ERROR_OK = 0;
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
     * Constructor.
     * @param view the owner XWalkView instance.
     */
    public XWalkResourceClient(XWalkView view) {
        // Keep the above parameter for future use.
    }

    /**
     * Notify the client that the XWalkView will load the resource specified
     * by the given url.
     * @param view the owner XWalkView instance.
     * @param url the url for the resource to be loaded.
     */
    public void onLoadStarted(XWalkView view, String url) {
    }

    /**
     * Notify the client that the XWalkView completes to load the resource
     * specified by the given url.
     * @param view the owner XWalkView instance.
     * @param url the url for the resource done for loading.
     */
    public void onLoadFinished(XWalkView view, String url) {
    }

    /**
     * Notify the client the progress info of loading a specific url.
     * @param view the owner XWalkView instance.
     * @param progressInPercent the loading process in percent.
     */
    public void onProgressChanged(XWalkView view, int progressInPercent) {
    }

    /**
     * Notify the client of a resource request and allow the client to return
     * the data.  If the return value is null, the XWalkView
     * will continue to load the resource as usual.  Otherwise, the return
     * response and data will be used.  NOTE: This method is called by the
     * network thread so clients should exercise caution when accessing private
     * data.
     * @param view The {@link org.xwalk.core.XWalkView} that is requesting the
     *             resource.
     * @param url The raw url of the resource.
     * @return A {@link android.webkit.WebResourceResponse} containing the
     *         response information or null if the XWalkView should load the
     *         resource itself.
     */
    public WebResourceResponse shouldInterceptLoadRequest(XWalkView view, String url) {
        return null;
    }

    /**
     * Report an error to the client.
     * @param view the owner XWalkView instance.
     * @param errorCode the error id.
     * @param description A String describing the error.
     * @param failingUrl The url that failed to load.
     */
    public void onReceivedLoadError(XWalkView view, int errorCode, String description,
            String failingUrl) {
        AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(view.getContext());
        dialogBuilder.setTitle(android.R.string.dialog_alert_title)
                .setMessage(description)
                .setCancelable(false)
                .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                    }
                });
        AlertDialog dialog = dialogBuilder.create();
        dialog.show();
    }
}
