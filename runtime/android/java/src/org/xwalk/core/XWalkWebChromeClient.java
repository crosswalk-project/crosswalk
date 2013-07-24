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

import android.content.pm.ActivityInfo;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Message;
import android.view.View;
import android.webkit.WebStorage;
import android.webkit.GeolocationPermissions;
import android.webkit.ConsoleMessage;
import android.webkit.ValueCallback;

public class XWalkWebChromeClient {

    /**
     * Tell the host application the current progress of loading a page.
     * @param view The XWalkView that initiated the callback.
     * @param newProgress Current page loading progress, represented by
     *                    an integer between 0 and 100.
     */
    public void onProgressChanged(XWalkView view, int newProgress) {}

    /**
     * Notify the host application of a change in the document title.
     * @param view The XWalkView that initiated the callback.
     * @param title A String containing the new title of the document.
     */
    public void onReceivedTitle(XWalkView view, String title) {}

    /**
     * Notify the host application of a new favicon for the current page.
     * @param view The XWalkView that initiated the callback.
     * @param icon A Bitmap containing the favicon for the current page.
     */
    public void onReceivedIcon(XWalkView view, Bitmap icon) {}

    /**
     * Notify the host application of the url for an apple-touch-icon.
     * @param view The XWalkView that initiated the callback.
     * @param url The icon url.
     * @param precomposed True if the url is for a precomposed touch icon.
     */
    public void onReceivedTouchIconUrl(XWalkView view, String url,
            boolean precomposed) {}

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
    public void onShowCustomView(View view, CustomViewCallback callback) {};

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
    public void onHideCustomView() {}

    /**
     * Request the host application to create a new window. If the host
     * application chooses to honor this request, it should return true from
     * this method, create a new XWalkView to host the window, insert it into the
     * View system and send the supplied resultMsg message to its target with
     * the new XWalkView as an argument. If the host application chooses not to
     * honor the request, it should return false from this method. The default
     * implementation of this method does nothing and hence returns false.
     * @param view The XWalkView from which the request for a new window
     *             originated.
     * @param isDialog True if the new window should be a dialog, rather than
     *                 a full-size window.
     * @param isUserGesture True if the request was initiated by a user gesture,
     *                      such as the user clicking a link.
     * @param resultMsg The message to send when once a new XWalkView has been
     *                  created. resultMsg.obj is a
     *                  {@link XWalkView.XWalkViewTransport} object. This should be
     *                  used to transport the new XWalkView, by calling
     *                  {@link XWalkView.XWalkViewTransport#setXWalkView(XWalkView)
     *                  XWalkView.XWalkViewTransport.setXWalkView(XWalkView)}.
     * @return This method should return true if the host application will
     *         create a new window, in which case resultMsg should be sent to
     *         its target. Otherwise, this method should return false. Returning
     *         false from this method but also sending resultMsg will result in
     *         undefined behavior.
     */
    public boolean onCreateWindow(XWalkView view, boolean isDialog,
            boolean isUserGesture, Message resultMsg) {
        return false;
    }

    /**
     * Request display and focus for this XWalkView. This may happen due to
     * another XWalkView opening a link in this XWalkView and requesting that this
     * XWalkView be displayed.
     * @param view The XWalkView that needs to be focused.
     */
    public void onRequestFocus(XWalkView view) {}

    /**
     * Notify the host application to close the given XWalkView and remove it
     * from the view system if necessary. At this point, WebCore has stopped
     * any loading in this window and has removed any cross-scripting ability
     * in javascript.
     * @param window The XWalkView that needs to be closed.
     */
    public void onCloseWindow(XWalkView window) {}

    /**
     * Tell the client to display a javascript alert dialog.  If the client
     * returns true, XWalkView will assume that the client will handle the
     * dialog.  If the client returns false, it will continue execution.
     * @param view The XWalkView that initiated the callback.
     * @param url The url of the page requesting the dialog.
     * @param message Message to be displayed in the window.
     * @param result A JsResult to confirm that the user hit enter.
     * @return boolean Whether the client will handle the alert dialog.
     */
    public boolean onJsAlert(XWalkView view, String url, String message,
            JsResult result) {
        return false;
    }

    /**
     * Tell the client to display a confirm dialog to the user. If the client
     * returns true, XWalkView will assume that the client will handle the
     * confirm dialog and call the appropriate JsResult method. If the
     * client returns false, a default value of false will be returned to
     * javascript. The default behavior is to return false.
     * @param view The XWalkView that initiated the callback.
     * @param url The url of the page requesting the dialog.
     * @param message Message to be displayed in the window.
     * @param result A JsResult used to send the user's response to
     *               javascript.
     * @return boolean Whether the client will handle the confirm dialog.
     */
    public boolean onJsConfirm(XWalkView view, String url, String message,
            JsResult result) {
        return false;
    }

    /**
     * Tell the client to display a prompt dialog to the user. If the client
     * returns true, XWalkView will assume that the client will handle the
     * prompt dialog and call the appropriate JsPromptResult method. If the
     * client returns false, a default value of false will be returned to to
     * javascript. The default behavior is to return false.
     * @param view The XWalkView that initiated the callback.
     * @param url The url of the page requesting the dialog.
     * @param message Message to be displayed in the window.
     * @param defaultValue The default value displayed in the prompt dialog.
     * @param result A JsPromptResult used to send the user's reponse to
     *               javascript.
     * @return boolean Whether the client will handle the prompt dialog.
     */
    public boolean onJsPrompt(XWalkView view, String url, String message,
            String defaultValue, JsPromptResult result) {
        return false;
    }

    /**
     * Tell the client to display a dialog to confirm navigation away from the
     * current page. This is the result of the onbeforeunload javascript event.
     * If the client returns true, XWalkView will assume that the client will
     * handle the confirm dialog and call the appropriate JsResult method. If
     * the client returns false, a default value of true will be returned to
     * javascript to accept navigation away from the current page. The default
     * behavior is to return false. Setting the JsResult to true will navigate
     * away from the current page, false will cancel the navigation.
     * @param view The XWalkView that initiated the callback.
     * @param url The url of the page requesting the dialog.
     * @param message Message to be displayed in the window.
     * @param result A JsResult used to send the user's response to
     *               javascript.
     * @return boolean Whether the client will handle the confirm dialog.
     */
    public boolean onJsBeforeUnload(XWalkView view, String url, String message,
            JsResult result) {
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
    *                     must be used to inform the XWalkView of the new quota.
    */
    // Note that the callback must always be executed at some point to ensure
    // that the sleeping WebCore thread is woken up.
    public void onExceededDatabaseQuota(String url, String databaseIdentifier,
            long quota, long estimatedDatabaseSize, long totalQuota,
            WebStorage.QuotaUpdater quotaUpdater) {
        // This default implementation passes the current quota back to WebCore.
        // WebCore will interpret this that new quota was declined.
        quotaUpdater.updateQuota(quota);
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
    *                     must be used to inform the XWalkView of the new quota.
    */
    // Note that the callback must always be executed at some point to ensure
    // that the sleeping WebCore thread is woken up.
    public void onReachedMaxAppCacheSize(long requiredStorage, long quota,
            WebStorage.QuotaUpdater quotaUpdater) {
        quotaUpdater.updateQuota(quota);
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
            GeolocationPermissions.Callback callback) {}

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
     * When not playing, video elements are represented by a 'poster' image. The
     * image to use can be specified by the poster attribute of the video tag in
     * HTML. If the attribute is absent, then a default poster will be used. This
     * method allows the ChromeClient to provide that default image.
     *
     * @return Bitmap The image to use as a default poster, or null if no such image is
     * available.
     */
    public Bitmap getDefaultVideoPoster() {
        return null;
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
     * Tell the client to open a file chooser.
     * @param uploadFile A ValueCallback to set the URI of the file to upload.
     *      onReceiveValue must be called to wake up the thread.a
     * @param acceptType The value of the 'accept' attribute of the input tag
     *         associated with this file picker.
     * @param capture The value of the 'capture' attribute of the input tag
     *         associated with this file picker.
     * @hide
     */
    public void openFileChooser(ValueCallback<Uri> uploadFile, String acceptType, String capture) {
        uploadFile.onReceiveValue(null);
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
