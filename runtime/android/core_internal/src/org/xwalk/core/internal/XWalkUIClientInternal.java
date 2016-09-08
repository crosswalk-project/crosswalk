// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.pm.ActivityInfo;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Build.VERSION;
import android.os.Build.VERSION_CODES;
import android.os.Message;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.webkit.ValueCallback;
import android.widget.EditText;
import android.widget.FrameLayout;

import org.chromium.base.ApiCompatibilityUtils;

/**
 * This class notifies the embedder UI events/callbacks.
 */
@XWalkAPI(createExternally = true)
public class XWalkUIClientInternal {

    private Context mContext;
    private AlertDialog mDialog;
    private EditText mPromptText;
    private int mSystemUiFlag;
    private XWalkViewInternal mXWalkView;
    private boolean mOriginalFullscreen;
    private boolean mOriginalForceNotFullscreen;
    private boolean mIsFullscreen = false;
    private View mCustomXWalkView;
    private final int INVALID_ORIENTATION = -2;
    private int mPreOrientation = INVALID_ORIENTATION;
    private CustomViewCallbackInternal mCustomViewCallback;
    private XWalkContentsClient mContentsClient;

    /**
     * Initiator
     * @since 4.0
     */
    @XWalkAPI
    public enum InitiateByInternal {
        BY_USER_GESTURE,
        BY_JAVASCRIPT
    }

    /**
     * Constructor.
     * @param view the owner XWalkViewInternal instance.
     * @since 1.0
     */
    @XWalkAPI
    public XWalkUIClientInternal(XWalkViewInternal view) {
        mContext = view.getContext();
        if (VERSION.SDK_INT >= VERSION_CODES.KITKAT) {
            mSystemUiFlag = View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
                    View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
                    View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN;
        }
        mXWalkView = view;
    }

    /**
     * Request the host application to create a new window
     * @param view The XWalkView which initiate the request for a new window
     * @param initiator How the request was initiated
     * @param callback Callback when once a new XWalkView has been created
     * @return Return true if the host application will create a new window
     * @since 4.0
     */
    @XWalkAPI
    public boolean onCreateWindowRequested(XWalkViewInternal view, InitiateByInternal initiator,
            ValueCallback<XWalkViewInternal> callback) {
        return false;
    }

    /**
     * Called when the theme color is changed. This works only on Android Lollipop+(5.0+).
     * @param color the new color in RGB format.
     */
    public void onDidChangeThemeColor(XWalkViewInternal view, int color) {
        if (view == null || !(mContext instanceof Activity)) return;
        Activity activity = (Activity) mContext;
        ApiCompatibilityUtils.setStatusBarColor(activity.getWindow(),color);
        ApiCompatibilityUtils.setTaskDescription(activity, null, null, color);
    }

    /**
     * Notify the host application that an icon is available, send the message to start the downloading
     * @param view The XWalkView that icon belongs to
     * @param url The icon url
     * @param startDownload Message to initiate icon download
     * @since 4.0
     */
    @XWalkAPI
    public void onIconAvailable(XWalkViewInternal view, String url, Message startDownload) {
    }

    /**
     * Notify the host application of a new icon has been downloaded
     * @param view The XWalkView that icon belongs to
     * @param url The icon url
     * @param icon The icon image
     * @since 4.0
     */
    @XWalkAPI
    public void onReceivedIcon(XWalkViewInternal view, String url, Bitmap icon) {
    }

    /**
     * Request display and focus for this XWalkViewInternal.
     * @param view the owner XWalkViewInternal instance.
     * @since 1.0
     */
    @XWalkAPI
    public void onRequestFocus(XWalkViewInternal view) {
    }

    /**
     * Notify the client to close the given XWalkViewInternal.
     * @param view the owner XWalkViewInternal instance.
     * @since 1.0
     */
    @XWalkAPI
    public void onJavascriptCloseWindow(XWalkViewInternal view) {
        if (view == null || !(mContext instanceof Activity)) return;
        Activity activity = (Activity) mContext;
        activity.finish();
    }

    /**
     * The type of JavaScript modal dialog.
     * @since 1.0
     */
    @XWalkAPI
    public enum JavascriptMessageTypeInternal {
        /** JavaScript alert dialog. */
        JAVASCRIPT_ALERT,
        /** JavaScript confirm dialog. */
        JAVASCRIPT_CONFIRM,
        /** JavaScript prompt dialog. */
        JAVASCRIPT_PROMPT,
        /** JavaScript dialog for a window-before-unload notification. */
        JAVASCRIPT_BEFOREUNLOAD
    }

    /**
     * Tell the client to display a prompt dialog to the user.
     * @param view the owner XWalkViewInternal instance.
     * @param type the type of JavaScript modal dialog.
     * @param url the url of the web page which wants to show this dialog.
     * @param message the message to be shown.
     * @param defaultValue the default value string. Only valid for Prompt dialog.
     * @param result the callback to handle the result from caller.
     * @return true if the client will handle the dialog
     * @since 1.0
     */
    @XWalkAPI
    public boolean onJavascriptModalDialog(XWalkViewInternal view, JavascriptMessageTypeInternal type,
            String url, String message, String defaultValue, XWalkJavascriptResultInternal result) {
        switch(type) {
            case JAVASCRIPT_ALERT:
                return onJsAlert(view, url, message, result);
            case JAVASCRIPT_CONFIRM:
                return onJsConfirm(view, url, message, result);
            case JAVASCRIPT_PROMPT:
                return onJsPrompt(view, url, message, defaultValue, result);
            case JAVASCRIPT_BEFOREUNLOAD:
                // Reuse onJsConfirm to show the dialog.
                return onJsConfirm(view, url, message, result);
            default:
                break;
        }
        assert(false);
        return false;
    }

    /**
     * Tell the client to toggle fullscreen mode.
     * @param view the owner XWalkViewInternal instance.
     * @param enterFullscreen true if it has entered fullscreen mode.
     * @since 1.0
     */
    @XWalkAPI
    public void onFullscreenToggled(XWalkViewInternal view, boolean enterFullscreen) {
        if (!(mContext instanceof Activity)) return;

        Activity activity = (Activity) mContext;
        if (enterFullscreen) {
            if ((activity.getWindow().getAttributes().flags &
                    WindowManager.LayoutParams.FLAG_FORCE_NOT_FULLSCREEN) != 0) {
                mOriginalForceNotFullscreen = true;
                activity.getWindow().clearFlags(
                        WindowManager.LayoutParams.FLAG_FORCE_NOT_FULLSCREEN);
            } else {
                mOriginalForceNotFullscreen = false;
            }
            if (!mIsFullscreen) {
                if (VERSION.SDK_INT >= VERSION_CODES.KITKAT) {
                    View decorView = activity.getWindow().getDecorView();
                    mSystemUiFlag = decorView.getSystemUiVisibility();
                    decorView.setSystemUiVisibility(
                            View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
                            View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
                            View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
                            View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
                            View.SYSTEM_UI_FLAG_FULLSCREEN |
                            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
                } else {
                    if ((activity.getWindow().getAttributes().flags &
                            WindowManager.LayoutParams.FLAG_FULLSCREEN) != 0) {
                        mOriginalFullscreen = true;
                    } else {
                        mOriginalFullscreen = false;
                        activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
                    }
                }
                mIsFullscreen = true;
            }
        } else {
            if (mOriginalForceNotFullscreen) {
                activity.getWindow().addFlags(
                        WindowManager.LayoutParams.FLAG_FORCE_NOT_FULLSCREEN);
            }
            if (VERSION.SDK_INT >= VERSION_CODES.KITKAT) {
                activity.getWindow().getDecorView().setSystemUiVisibility(mSystemUiFlag);
            } else {
                // Clear the activity fullscreen flag.
                if (!mOriginalFullscreen) {
                    activity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
                }
            }
            mIsFullscreen = false;
        }
    }

    /**
     * Tell the client to open a file chooser.
     * @param view the owner XWalkViewInternal instance.
     * @param uploadFile the callback class to handle the result from caller. It MUST
     *        be invoked in all cases. Leave it not invoked will block all following
     *        requests to open file chooser.
     * @param acceptType value of the 'accept' attribute of the input tag associated
     *        with this file picker.
     * @param capture value of the 'capture' attribute of the input tag associated
     *        with this file picker
     * @since 1.0
     */
    @XWalkAPI
    public void openFileChooser(XWalkViewInternal view, ValueCallback<Uri> uploadFile,
            String acceptType, String capture) {
        uploadFile.onReceiveValue(null);
    }

    /**
     * Notify the client that the scale applied to the XWalkViewInternal has changed.
     * @param view the owner XWalkViewInternal instance.
     * @param oldScale the old scale before scaling.
     * @param newScale the current scale factor after scaling.
     * @since 1.0
     */
    @XWalkAPI
    public void onScaleChanged(XWalkViewInternal view, float oldScale, float newScale) {
    }

    /**
     * Give the host application a chance to handle the key event synchronously.
     * e.g. menu shortcut key events need to be filtered this way. If return
     * true, XWalkViewInternal will not handle the key event. If return false, XWalkViewInternal
     * will always handle the key event, so none of the super in the view chain
     * will see the key event. The default behavior returns false.
     *
     * @param view The XWalkViewInternal that is initiating the callback.
     * @param event The key event.
     * @return True if the host application wants to handle the key event
     *         itself, otherwise return false
     *
     * @since 2.1
     */
    @XWalkAPI
    public boolean shouldOverrideKeyEvent(XWalkViewInternal view, KeyEvent event) {
        return false;
    }

    /**
     * Notify the host application that a key was not handled by the XWalkViewInternal.
     * Except system keys, XWalkViewInternal always consumes the keys in the normal flow
     * or if shouldOverrideKeyEvent returns true. This is called asynchronously
     * from where the key is dispatched. It gives the host application a chance
     * to handle the unhandled key events.
     *
     * @param view The XWalkViewInternal that is initiating the callback.
     * @param event The key event.
     *
     * @since 2.1
     */
    @XWalkAPI
    public void onUnhandledKeyEvent(XWalkViewInternal view, KeyEvent event) {
    }

    /**
     * Initiator
     * @since 5.0
     */
    @XWalkAPI
    public enum ConsoleMessageType {
        DEBUG,
        ERROR,
        LOG,
        INFO,
        WARNING
    }

    /**
     * Notify the host application of console message.
     * @param view The XWalkViewInternal that initiated the callback.
     * @param message A String containing the console message.
     * @param lineNumber The line number of JavaScript.
     * @param sourceId The link which print log.
     * @param messageType The type of console message.
     * @return Not applicable here. For future use.
     * @since 5.0
     */
    @XWalkAPI
    public boolean onConsoleMessage(XWalkViewInternal view, String message,
            int lineNumber, String sourceId, ConsoleMessageType messageType) {
        return false;
    }

    /**
     * Notify the host application of a change in the document title.
     * @param view The XWalkViewInternal that initiated the callback.
     * @param title A String containing the new title of the document.
     * @since 2.1
     */
    @XWalkAPI
    public void onReceivedTitle(XWalkViewInternal view, String title) {
    }


    /**
     * The status when a page stopped loading
     * @since 2.1
     */
    @XWalkAPI
    public enum LoadStatusInternal {
        /** Loading finished. */
        FINISHED,
        /** Loading failed. */
        FAILED,
        /** Loading cancelled by user. */
        CANCELLED
    }

    /**
     * Notify the host application that a page has started loading. This method
     * is called once for each main frame load so a page with iframes or
     * framesets will call onPageLoadStarted one time for the main frame. This also
     * means that onPageLoadStarted will not be called when the contents of an
     * embedded frame changes, i.e. clicking a link whose target is an iframe.
     *
     * @param view The XWalkViewInternal that is initiating the callback.
     * @param url The url to be loaded.
     *
     * @since 2.1
     */
    @XWalkAPI
    public void onPageLoadStarted(XWalkViewInternal view, String url) {
    }

    /**
     * Notify the host application that a page has stopped loading. This method
     * is called only for main frame. When onPageLoadStopped() is called, the
     * rendering picture may not be updated yet.
     *
     * @param view The XWalkViewInternal that is initiating the callback.
     * @param url The url of the page.
     * @param status The status when the page stopped loading.
     *
     * @since 2.1
     */
    @XWalkAPI
    public void onPageLoadStopped(XWalkViewInternal view, String url, LoadStatusInternal status) {
    }

    /**
     * Tell the client to display an alert dialog to the user.
     * WARN: Please DO NOT override this API and onJavascriptModalDialog API in the
     *       same subclass to avoid unexpected behavior.
     * @param view the owner XWalkViewInternal instance.
     * @param url the url of the web page which wants to show this dialog.
     * @param message the message to be shown.
     * @param result the callback to handle the result from caller.
     * @return Whether the client will handle the alert dialog.
     * @since 6.0
     */
    @XWalkAPI
    public boolean onJsAlert(XWalkViewInternal view, String url, String message,
            XWalkJavascriptResultInternal result) {
        final XWalkJavascriptResultInternal fResult = result;
        AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(mContext);
        dialogBuilder.setTitle(mContext.getString(R.string.js_alert_title))
                .setMessage(message)
                .setCancelable(true)
                .setPositiveButton(mContext.getString(android.R.string.ok),
                        new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        fResult.confirm();
                        dialog.dismiss();
                    }
                })
                .setOnCancelListener(new DialogInterface.OnCancelListener() {
                    @Override
                    public void onCancel(DialogInterface dialog) {
                        fResult.cancel();
                    }
                });
        mDialog = dialogBuilder.create();
        mDialog.show();
        return false;
    }

    /**
     * Tell the client to display a confirm dialog to the user.
     * WARN: Please DO NOT override this API and onJavascriptModalDialog API in the
     *       same subclass to avoid unexpected behavior.
     * @param view the owner XWalkViewInternal instance.
     * @param url the url of the web page which wants to show this dialog.
     * @param message the message to be shown.
     * @param result the callback to handle the result from caller.
     * @return Whether the client will handle the confirm dialog.
     * @since 6.0
     */
    @XWalkAPI
    public boolean onJsConfirm(XWalkViewInternal view, String url, String message,
            XWalkJavascriptResultInternal result) {
        final XWalkJavascriptResultInternal fResult = result;
        AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(mContext);
        dialogBuilder.setTitle(mContext.getString(R.string.js_confirm_title))
                .setMessage(message)
                .setCancelable(true)
                .setPositiveButton(mContext.getString(android.R.string.ok),
                        new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        fResult.confirm();
                        dialog.dismiss();
                    }
                })
                // Need to implement 'onClick' and call the dialog.cancel. Otherwise, the
                // UI will be locked.
                .setNegativeButton(mContext.getString(android.R.string.cancel),
                        new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        // This will call OnCancelLisitener.onCancel().
                        dialog.cancel();
                    }
                })
                .setOnCancelListener(new DialogInterface.OnCancelListener() {
                    @Override
                    public void onCancel(DialogInterface dialog) {
                        fResult.cancel();
                    }
                });
        mDialog = dialogBuilder.create();
        mDialog.show();
        return false;
    }

    /**
     * Tell the client to display a prompt dialog to the user.
     * WARN: Please DO NOT override this API and onJavascriptModalDialog API in the
     *       same subclass to avoid unexpected behavior.
     * @param view the owner XWalkViewInternal instance.
     * @param url the url of the web page which wants to show this dialog.
     * @param message the message to be shown.
     * @param defaultValue the default value string. Only valid for Prompt dialog.
     * @param result the callback to handle the result from caller.
     * @return Whether the client will handle the prompt dialog.
     * @since 6.0
     */
    @XWalkAPI
    public boolean onJsPrompt(XWalkViewInternal view, String url, String message,
            String defaultValue, XWalkJavascriptResultInternal result) {
        final XWalkJavascriptResultInternal fResult = result;
        AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(mContext);
        dialogBuilder.setTitle(mContext.getString(R.string.js_prompt_title))
                .setMessage(message)
                .setPositiveButton(mContext.getString(android.R.string.ok),
                        new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        fResult.confirmWithResult(mPromptText.getText().toString());
                        dialog.dismiss();
                    }
                })
                // Need to implement 'onClick' and call the dialog.cancel. Otherwise, the
                // UI will be locked.
                .setNegativeButton(mContext.getString(android.R.string.cancel),
                        new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        // This will call OnCancelLisitener.onCancel().
                        dialog.cancel();
                    }
                })
                .setOnCancelListener(new DialogInterface.OnCancelListener() {
                    @Override
                    public void onCancel(DialogInterface dialog) {
                        fResult.cancel();
                    }
                });
        mPromptText = new EditText(mContext);
        mPromptText.setVisibility(View.VISIBLE);
        mPromptText.setText(defaultValue);
        mPromptText.selectAll();

        dialogBuilder.setView(mPromptText);
        mDialog = dialogBuilder.create();
        mDialog.show();
        return false;
    }

    void setContentsClient(XWalkContentsClient client) {
        mContentsClient = client;
    }

    private Activity addContentView(View view, CustomViewCallbackInternal callback) {
        Activity activity = null;
        try {
            Context context = mXWalkView.getContext();
            if (context instanceof Activity) {
                activity = (Activity) context;
            }
        } catch (ClassCastException e) {
        }

        if (mCustomXWalkView != null || activity == null) {
            if (callback != null) callback.onCustomViewHidden();
            return null;
        }

        mCustomXWalkView = view;
        mCustomViewCallback = callback;
        if (mContentsClient != null) {
            mContentsClient.onToggleFullscreen(true);
        }

        // Add the video view to the activity's DecorView.
        FrameLayout decor = (FrameLayout) activity.getWindow().getDecorView();
        decor.addView(mCustomXWalkView, 0,
                new FrameLayout.LayoutParams(
                        ViewGroup.LayoutParams.MATCH_PARENT,
                        ViewGroup.LayoutParams.MATCH_PARENT,
                        Gravity.CENTER));
        return activity;
    }

    /**
     * Notify the host application that the current page would
     * like to show a custom View.
     * @param view is the View object to be shown.
     * @param callback is the callback to be invoked if and when the view is dismissed.
     * @since 7.0
     */
    @XWalkAPI
    public void onShowCustomView(View view,
            CustomViewCallbackInternal callback) {
        addContentView(view, callback);
    }

    /**
     * Notify the host application that the current page would
     * like to show a custom View in a particular orientation.
     * @param view is the View object to be shown.
     * @param requestedOrientation An orientation constant as used in
     * {@link ActivityInfo#screenOrientation ActivityInfo.screenOrientation}.
     * @param callback is the callback to be invoked if and when the view is dismissed.
     * @since 7.0
     */
    @XWalkAPI
    public void onShowCustomView(View view,
            int requestedOrientation, CustomViewCallbackInternal callback) {
        Activity activity = addContentView(view, callback);
        if (activity == null) return;

        final int orientation = activity.getResources().getConfiguration().orientation;

        if (requestedOrientation != orientation &&
                requestedOrientation >= ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED &&
                requestedOrientation <= ActivityInfo.SCREEN_ORIENTATION_LOCKED) {
            mPreOrientation = orientation;
            activity.setRequestedOrientation(requestedOrientation);
        }
    }

    /**
     * Notify the host application that the current page would
     * like to hide its custom view.
     * @since 7.0
     */
    @XWalkAPI
    public void onHideCustomView() {
        if (mCustomXWalkView == null || !(mXWalkView.getContext() instanceof Activity)) return;

        if (mContentsClient != null) {
            mContentsClient.onToggleFullscreen(false);
        }

        Activity activity = (Activity) mXWalkView.getContext();
        // Remove video view from activity's ContentView.
        FrameLayout decor = (FrameLayout) activity.getWindow().getDecorView();
        decor.removeView(mCustomXWalkView);
        if (mCustomViewCallback != null) mCustomViewCallback.onCustomViewHidden();

        if (mPreOrientation != INVALID_ORIENTATION &&
                mPreOrientation >= ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED &&
                mPreOrientation <= ActivityInfo.SCREEN_ORIENTATION_LOCKED) {
            activity.setRequestedOrientation(mPreOrientation);
            mPreOrientation = INVALID_ORIENTATION;
        }

        mCustomXWalkView = null;
        mCustomViewCallback = null;
    }
}
