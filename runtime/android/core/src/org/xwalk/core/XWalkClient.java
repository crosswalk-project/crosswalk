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

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.graphics.Bitmap;
import android.net.http.SslError;
import android.os.Message;
import android.view.KeyEvent;
import android.webkit.ValueCallback;
import android.webkit.WebResourceResponse;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.TextView;

/**
 * It's the Internal class to handle legacy resource related callbacks not
 * handled by XWalkResourceClient.
 */
public class XWalkClient {

    private Context mContext;
    private AlertDialog mDialog;
    private XWalkView mXWalkView;

    public XWalkClient(Context context, XWalkView view) {
        mContext = context;
        mXWalkView = view;
    }

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
     */
    public void onPageStarted(XWalkView view, String url) {
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
     * @param callback The callback class. Passing 'true' means accepting the
     *                 ssl error and continue to load. Passing 'false' means
     *                 forbidding to load the web page.
     * @param error The SSL error object.
     */
    public void onReceivedSslError(XWalkView view, ValueCallback<Boolean> callback,
            SslError error) {
        final ValueCallback<Boolean> valueCallback = callback;
        AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(mContext);
        // Don't use setOnDismissListener as it requires API level 17.
        dialogBuilder.setTitle(R.string.ssl_alert_title)
                .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        valueCallback.onReceiveValue(true);
                        dialog.dismiss();
                    }
                }).setNegativeButton(android.R.string.cancel, null)
                .setOnCancelListener(new DialogInterface.OnCancelListener() {
                    public void onCancel(DialogInterface dialog) {
                        valueCallback.onReceiveValue(false);
                    }
                });
        mDialog = dialogBuilder.create();
        mDialog.show();
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
        if (view == null) return;

        final XWalkHttpAuthHandler haHandler = handler;
        LinearLayout layout = new LinearLayout(mContext);
        final EditText userNameEditText = new EditText(mContext);
        final EditText passwordEditText = new EditText(mContext);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setPaddingRelative(10, 0, 10, 20);
        userNameEditText.setHint(R.string.http_auth_user_name);
        passwordEditText.setHint(R.string.http_auth_password);
        layout.addView(userNameEditText);
        layout.addView(passwordEditText);

        final Activity curActivity = mXWalkView.getActivity();
        AlertDialog.Builder httpAuthDialog = new AlertDialog.Builder(curActivity);
        httpAuthDialog.setTitle(R.string.http_auth_title)
                .setView(layout)
                .setCancelable(false)
                .setPositiveButton(R.string.http_auth_log_in, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        String userName = userNameEditText.getText().toString();
                        String password = passwordEditText.getText().toString();
                        haHandler.proceed(userName, password);
                        dialog.dismiss();
                    }
                }).setNegativeButton(android.R.string.cancel, null)
                .setOnCancelListener(new DialogInterface.OnCancelListener() {
                    public void onCancel(DialogInterface dialog) {
                        haHandler.cancel();
                    }
                }).create().show();
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

    // TODO(yongsheng): legacy method. Consider removing it?
    public void onLoadResource(XWalkView view, String url) {
    }
}
