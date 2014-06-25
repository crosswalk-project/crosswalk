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

package org.xwalk.core.internal;

import android.app.Activity;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Message;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.WebStorage;
import android.webkit.ConsoleMessage;
import android.webkit.ValueCallback;
import android.widget.FrameLayout;

/**
 * It's an internal legacy class which is to handle kinds of ui related
 * callback functions. It only handles those which are not exposed to
 * external users compared to XWalkUIClientInternal.
 *
 * @hide
 */
public class XWalkWebChromeClient {
    private Context mContext;
    private View mCustomXWalkView;
    private XWalkViewInternal mXWalkView;
    private XWalkWebChromeClient.CustomViewCallback mCustomViewCallback;
    private XWalkContentsClient mContentsClient = null;
    private long XWALK_MAX_QUOTA = 1024 * 1024 * 100;

    public XWalkWebChromeClient(XWalkViewInternal view) {
        mContext = view.getContext();
        mXWalkView = view;
    }

    void setContentsClient(XWalkContentsClient client) {
        mContentsClient = client;
    }

    /**
     * Notify the host application of a change in the document title.
     * @param view The XWalkViewInternal that initiated the callback.
     * @param title A String containing the new title of the document.
     */
    public void onReceivedTitle(XWalkViewInternal view, String title) {}

    /**
     * Notify the host application of a new favicon for the current page.
     * @param view The XWalkViewInternal that initiated the callback.
     * @param icon A Bitmap containing the favicon for the current page.
     */
    public void onReceivedIcon(XWalkViewInternal view, Bitmap icon) {}

    /**
     * A callback interface used by the host application to notify
     * the current page that its custom view has been dismissed.
     */
    public interface CustomViewCallback {
        /**
         * Invoked when the host application dismisses the
         * custom view.
         */
        public void onCustomViewHidden();
    }

    /**
     * Notify the host application that the current page would
     * like to show a custom View.
     * @param view is the View object to be shown.
     * @param callback is the callback to be invoked if and when the view
     * is dismissed.
     */
    public void onShowCustomView(View view, CustomViewCallback callback) {
        Activity activity = mXWalkView.getActivity();

        if (mCustomXWalkView != null || activity == null) {
            callback.onCustomViewHidden();
            return;
        }

        mCustomXWalkView = view;
        mCustomViewCallback = callback;

        if (mContentsClient != null) {
            mContentsClient.onToggleFullscreen(true);
        }

        // Add the video view to the activity's ContentView.
        activity.getWindow().addContentView(view,
                new FrameLayout.LayoutParams(
                        ViewGroup.LayoutParams.MATCH_PARENT,
                        ViewGroup.LayoutParams.MATCH_PARENT,
                        Gravity.CENTER));
    }

    /**
     * Notify the host application that the current page would
     * like to show a custom View in a particular orientation.
     * @param view is the View object to be shown.
     * @param requestedOrientation An orientation constant as used in
     * {@link ActivityInfo#screenOrientation ActivityInfo.screenOrientation}.
     * @param callback is the callback to be invoked if and when the view
     * is dismissed.
     */
    public void onShowCustomView(View view, int requestedOrientation,
            CustomViewCallback callback) {};

    /**
     * Notify the host application that the current page would
     * like to hide its custom view.
     */
    public void onHideCustomView() {
        Activity activity = mXWalkView.getActivity();

        if (mCustomXWalkView == null || activity == null) return;

        if (mContentsClient != null) {
            mContentsClient.onToggleFullscreen(true);
        }

        // Remove video view from activity's ContentView.
        FrameLayout decor = (FrameLayout) activity.getWindow().getDecorView();
        decor.removeView(mCustomXWalkView);
        mCustomViewCallback.onCustomViewHidden();

        mCustomXWalkView = null;
        mCustomViewCallback = null;
    }

    /**
     * Request the host application to create a new window. If the host
     * application chooses to honor this request, it should return true from
     * this method, create a new XWalkViewInternal to host the window, insert it into the
     * View system and send the supplied resultMsg message to its target with
     * the new XWalkViewInternal as an argument. If the host application chooses not to
     * honor the request, it should return false from this method. The default
     * implementation of this method does nothing and hence returns false.
     * @param view The XWalkViewInternal from which the request for a new window
     *             originated.
     * @param isDialog True if the new window should be a dialog, rather than
     *                 a full-size window.
     * @param isUserGesture True if the request was initiated by a user gesture,
     *                      such as the user clicking a link.
     * @param resultMsg The message to send when once a new XWalkViewInternal has been
     *                  created. resultMsg.obj is a
     *                  {@link XWalkViewInternal.XWalkViewTransport} object. This should be
     *                  used to transport the new XWalkViewInternal, by calling
     *                  {@link XWalkViewInternal.XWalkViewTransport#setXWalkView(XWalkViewInternal)
     *                  XWalkViewInternal.XWalkViewTransport.setXWalkView(XWalkViewInternal)}.
     * @return This method should return true if the host application will
     *         create a new window, in which case resultMsg should be sent to
     *         its target. Otherwise, this method should return false. Returning
     *         false from this method but also sending resultMsg will result in
     *         undefined behavior.
     */
    public boolean onCreateWindow(XWalkViewInternal view, boolean isDialog,
            boolean isUserGesture, Message resultMsg) {
        return false;
    }

   /**
    * Tell the client that the quota has been exceeded for the Web SQL Database
    * API for a particular origin and request a new quota. The client must
    * respond by invoking the
    * {@link WebStorage.QuotaUpdater#updateQuota(long) updateQuota(long)}
    * method of the supplied {@link WebStorage.QuotaUpdater} instance. The
    * minimum value that can be set for the new quota is the current quota. The
    * default implementation responds with the current quota, so the quota will
    * not be increased.
    * @param url The URL of the page that triggered the notification
    * @param databaseIdentifier The identifier of the database where the quota
    *                           was exceeded.
    * @param quota The quota for the origin, in bytes
    * @param estimatedDatabaseSize The estimated size of the offending
    *                              database, in bytes
    * @param totalQuota The total quota for all origins, in bytes
    * @param quotaUpdater An instance of {@link WebStorage.QuotaUpdater} which
    *                     must be used to inform the XWalkViewInternal of the new quota.
    */
    // Note that the callback must always be executed at some point to ensure
    // that the sleeping WebCore thread is woken up.
    // Since the parameter type WebStorage.QuotaUpdater and this API are
    // deprecated in Android 4.4, while this parameter type and this API
    // are still used before Android 4.4, no other API and parameter are
    // to replace them, suppress the compiling warnings for Android 4.4
    // due to deprecation.
    @SuppressWarnings("deprecation")
    public void onExceededDatabaseQuota(String url, String databaseIdentifier,
            long quota, long estimatedDatabaseSize, long totalQuota,
            WebStorage.QuotaUpdater quotaUpdater) {
        // This default implementation passes the current quota back to WebCore.
        // WebCore will interpret this that new quota was declined.
        quotaUpdater.updateQuota(XWALK_MAX_QUOTA);
    }

   /**
    * Tell the client that the quota has been reached for the Application Cache
    * API and request a new quota. The client must respond by invoking the
    * {@link WebStorage.QuotaUpdater#updateQuota(long) updateQuota(long)}
    * method of the supplied {@link WebStorage.QuotaUpdater} instance. The
    * minimum value that can be set for the new quota is the current quota. The
    * default implementation responds with the current quota, so the quota will
    * not be increased.
    * @param requiredStorage The amount of storage required by the Application
    *                        Cache operation that triggered this notification,
    *                        in bytes.
    * @param quota The quota, in bytes
    * @param quotaUpdater An instance of {@link WebStorage.QuotaUpdater} which
    *                     must be used to inform the XWalkViewInternal of the new quota.
    */
    // Note that the callback must always be executed at some point to ensure
    // that the sleeping WebCore thread is woken up.
    // Since the parameter type WebStorage.QuotaUpdater and this API are
    // deprecated in Android 4.4, while this parameter type and this API
    // are still used before Android 4.4, no other API and parameter are
    // to replace them, suppress the compiling warnings for Android 4.4
    // due to deprecation.
    @SuppressWarnings("deprecation")
    public void onReachedMaxAppCacheSize(long requiredStorage, long quota,
            WebStorage.QuotaUpdater quotaUpdater) {
        quotaUpdater.updateQuota(XWALK_MAX_QUOTA);
    }

    /**
     * Notify the host application that web content from the specified origin
     * is attempting to use the Geolocation API, but no permission state is
     * currently set for that origin. The host application should invoke the
     * specified callback with the desired permission state. See
     * {@link GeolocationPermissions} for details.
     * @param origin The origin of the web content attempting to use the
     *               Geolocation API.
     * @param callback The callback to use to set the permission state for the
     *                 origin.
     */
    public void onGeolocationPermissionsShowPrompt(String origin,
            XWalkGeolocationPermissions.Callback callback) {
        // Allow all origins for geolocation requests here for Crosswalk.
        // TODO(yongsheng): Need to define a UI prompt?
        callback.invoke(origin, true, false);
    }

    /**
     * Notify the host application that a request for Geolocation permissions,
     * made with a previous call to
     * {@link #onGeolocationPermissionsShowPrompt(String,GeolocationPermissions.Callback) onGeolocationPermissionsShowPrompt()}
     * has been canceled. Any related UI should therefore be hidden.
     */
    public void onGeolocationPermissionsHidePrompt() {}

    /**
     * Tell the client that a JavaScript execution timeout has occured. And the
     * client may decide whether or not to interrupt the execution. If the
     * client returns true, the JavaScript will be interrupted. If the client
     * returns false, the execution will continue. Note that in the case of
     * continuing execution, the timeout counter will be reset, and the callback
     * will continue to occur if the script does not finish at the next check
     * point.
     * @return boolean Whether the JavaScript execution should be interrupted.
     */
    public boolean onJsTimeout() {
        return true;
    }

    /**
     * Report a JavaScript error message to the host application. The ChromeClient
     * should override this to process the log message as they see fit.
     * @param message The error message to report.
     * @param lineNumber The line number of the error.
     * @param sourceID The name of the source file that caused the error.
     * @deprecated Use {@link #onConsoleMessage(ConsoleMessage) onConsoleMessage(ConsoleMessage)}
     *      instead.
     */
    @Deprecated
    public void onConsoleMessage(String message, int lineNumber, String sourceID) { }

    /**
     * Report a JavaScript console message to the host application. The ChromeClient
     * should override this to process the log message as they see fit.
     * @param consoleMessage Object containing details of the console message.
     * @return true if the message is handled by the client.
     */
    public boolean onConsoleMessage(ConsoleMessage consoleMessage) {
        // Call the old version of this function for backwards compatability.
        onConsoleMessage(consoleMessage.message(), consoleMessage.lineNumber(),
                consoleMessage.sourceId());
        return false;
    }

    /**
     * When the user starts to playback a video element, it may take time for enough
     * data to be buffered before the first frames can be rendered. While this buffering
     * is taking place, the ChromeClient can use this function to provide a View to be
     * displayed. For example, the ChromeClient could show a spinner animation.
     *
     * @return View The View to be displayed whilst the video is loading.
     */
    public View getVideoLoadingProgressView() {
        return null;
    }

    /** Obtains a list of all visited history items, used for link coloring
     */
    public void getVisitedHistory(ValueCallback<String[]> callback) {
    }

    /**
     * Tell the client that the page being viewed is web app capable,
     * i.e. has specified the fullscreen-web-app-capable meta tag.
     * @hide
     */
    public void setInstallableWebApp() { }

    /**
     * Tell the client that the page being viewed has an autofillable
     * form and the user would like to set a profile up.
     * @param msg A Message to send once the user has successfully
     *      set up a profile and to inform the WebTextView it should
     *      now autofill using that new profile.
     * @hide
     */
    public void setupAutoFill(Message msg) { }
}
