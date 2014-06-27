// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.webkit.WebResourceResponse;

import org.xwalk.core.internal.XWalkResourceClientInternal;
import org.xwalk.core.internal.XWalkViewInternal;

/**
 * This class notifies the embedder resource events/callbacks.
 */
public class XWalkResourceClient extends XWalkResourceClientInternal {
    /**
     * Success
     * @since 1.0
     */
    public static final int ERROR_OK = 0;
    /**
     * Generic error
     * @since 1.0
     */
    public static final int ERROR_UNKNOWN = -1;
    /**
     * Server or proxy hostname lookup failed
     * @since 1.0
     */
    public static final int ERROR_HOST_LOOKUP = -2;
    /**
     * Unsupported authentication scheme (not basic or digest)
     * @since 1.0
     */
    public static final int ERROR_UNSUPPORTED_AUTH_SCHEME = -3;
    /**
     * User authentication failed on server
     * @since 1.0
     */
    public static final int ERROR_AUTHENTICATION = -4;
    /**
     * User authentication failed on proxy
     * @since 1.0
     */
    public static final int ERROR_PROXY_AUTHENTICATION = -5;
    /**
     * Failed to connect to the server
     * @since 1.0
     */
    public static final int ERROR_CONNECT = -6;
    /**
     * Failed to read or write to the server
     * @since 1.0
     */
    public static final int ERROR_IO = -7;
    /**
     * Connection timed out
     * @since 1.0
     */
    public static final int ERROR_TIMEOUT = -8;
    /**
     * Too many redirects
     * @since 1.0
     */
    public static final int ERROR_REDIRECT_LOOP = -9;
    /**
     * Unsupported URI scheme
     * @since 1.0
     */
    public static final int ERROR_UNSUPPORTED_SCHEME = -10;
    /**
     * Failed to perform SSL handshake
     * @since 1.0
     */
    public static final int ERROR_FAILED_SSL_HANDSHAKE = -11;
    /**
     * Malformed URL
     * @since 1.0
     */
    public static final int ERROR_BAD_URL = -12;
    /**
     * Generic file error
     * @since 1.0
     */
    public static final int ERROR_FILE = -13;
    /**
     * File not found
     * @since 1.0
     */
    public static final int ERROR_FILE_NOT_FOUND = -14;
    /**
     * Too many requests during this load
     * @since 1.0
     */
    public static final int ERROR_TOO_MANY_REQUESTS = -15;

    /**
     * Constructor.
     * @param view the owner XWalkView instance.
     * @since 1.0
     */
    public XWalkResourceClient(XWalkView view) {
        super(view);
    }

    /**
     * Notify the client that the XWalkView will load the resource specified
     * by the given url.
     * @param view the owner XWalkView instance.
     * @param url the url for the resource to be loaded.
     * @since 1.0
     */
    public void onLoadStarted(XWalkView view, String url) {
        super.onLoadStarted(view, url);
    }

    /**
     * @hide
     */
    @Override
    public void onLoadStarted(XWalkViewInternal view, String url) {
        if (view instanceof XWalkView) {
            onLoadStarted((XWalkView) view, url);
        } else {
            super.onLoadStarted(view, url);
        }
    }

    /**
     * Notify the client that the XWalkView completes to load the resource
     * specified by the given url.
     * @param view the owner XWalkView instance.
     * @param url the url for the resource done for loading.
     * @since 1.0
     */
    public void onLoadFinished(XWalkView view, String url) {
        super.onLoadFinished(view, url);
    }

    /**
     * @hide
     */
    @Override
    public void onLoadFinished(XWalkViewInternal view, String url) {
        if (view instanceof XWalkView) {
            onLoadFinished((XWalkView) view, url);
        } else {
            super.onLoadFinished(view, url);
        }
    }

    /**
     * Notify the client the progress info of loading a specific url.
     * @param view the owner XWalkView instance.
     * @param progressInPercent the loading process in percent.
     * @since 1.0
     */
    public void onProgressChanged(XWalkView view, int progressInPercent) {
        super.onProgressChanged(view, progressInPercent);
    }

    /**
     * @hide
     */
    @Override
    public void onProgressChanged(XWalkViewInternal view, int progressInPercent) {
        if (view instanceof XWalkView) {
            onProgressChanged((XWalkView) view, progressInPercent);
        } else {
            super.onProgressChanged(view, progressInPercent);
        }
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
     * @since 1.0
     */
    public WebResourceResponse shouldInterceptLoadRequest(XWalkView view, String url) {
        return super.shouldInterceptLoadRequest(view, url);
    }

    /**
     * @hide
     */
    @Override
    public WebResourceResponse shouldInterceptLoadRequest(XWalkViewInternal view, String url) {
        if (view instanceof XWalkView) {
            return shouldInterceptLoadRequest((XWalkView) view, url);
        }

        return super.shouldInterceptLoadRequest(view, url);
    }

    /**
     * Report an error to the client.
     * @param view the owner XWalkView instance.
     * @param errorCode the error id.
     * @param description A String describing the error.
     * @param failingUrl The url that failed to load.
     * @since 1.0
     */
    public void onReceivedLoadError(XWalkView view, int errorCode, String description,
            String failingUrl) {
        super.onReceivedLoadError(view, errorCode, description, failingUrl);
    }

    /**
     * @hide
     */
    @Override
    public void onReceivedLoadError(XWalkViewInternal view, int errorCode, String description,
            String failingUrl) {
        if (view instanceof XWalkView) {
            onReceivedLoadError((XWalkView) view, errorCode, description, failingUrl);
        } else {
            super.onReceivedLoadError(view, errorCode, description, failingUrl);
        }
    }
}
