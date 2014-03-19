// Copyright (c) 2013 Intel Corporation. All rights reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.net.Uri;
import android.os.Build.VERSION;
import android.os.Build.VERSION_CODES;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.webkit.ValueCallback;
import android.widget.EditText;

/**
 * This is the implementation of XWalkUIClient if callers don't specify one.
 */
public class XWalkUIClientImpl extends XWalkUIClient {

    // Strings for displaying Dialog.
    private static String mJSAlertTitle;
    private static String mJSConfirmTitle;
    private static String mJSPromptTitle;
    private static String mOKButton;
    private static String mCancelButton;

    private Context mContext;
    private AlertDialog mDialog;
    private EditText mPromptText;
    private int mSystemUiFlag;
    private View mDecorView;
    private XWalkView mXWalkView;
    private boolean mOriginalFullscreen;

    public XWalkUIClientImpl(Context context, XWalkView view) {
        mContext = context;
        mDecorView = view.getActivity().getWindow().getDecorView();
        if (VERSION.SDK_INT >= VERSION_CODES.KITKAT) {
            mSystemUiFlag = View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
                    View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
                    View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN;
        }
        mXWalkView = view;
        initResources(context);
    }

    private static void initResources(Context context) {
        if (mJSAlertTitle != null) return;
        mJSAlertTitle = context.getString(R.string.js_alert_title);
        mJSConfirmTitle = context.getString(R.string.js_confirm_title);
        mJSPromptTitle = context.getString(R.string.js_prompt_title);
        mOKButton = context.getString(android.R.string.ok);
        mCancelButton = context.getString(android.R.string.cancel);
    }

    @Override
    public void onJavascriptCloseWindow(XWalkView view) {
        if (view != null && view.getActivity() != null) {
            view.getActivity().finish();
        }
    }

    @Override
    public boolean onJavascriptModalDialog(XWalkView view, JavascriptMessageType type, String url,
            String message, String defaultValue, XWalkJavascriptResult result) {
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

    private boolean onJsAlert(XWalkView view, String url, String message,
            XWalkJavascriptResult result) {
        final XWalkJavascriptResult fResult = result;
        AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(mContext);
        dialogBuilder.setTitle(mJSAlertTitle)
                .setMessage(message)
                .setCancelable(true)
                .setPositiveButton(mOKButton, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        fResult.confirm();
                        dialog.dismiss();
                    }
                }).setOnKeyListener(new DialogInterface.OnKeyListener() {
                    @Override
                    public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                        if (keyCode == KeyEvent.KEYCODE_BACK) {
                            fResult.confirm();
                            return false;
                        } else {
                            return true;
                        }
                    }
                });
        mDialog = dialogBuilder.create();
        mDialog.show();
        return false;
    }

    private boolean onJsConfirm(XWalkView view, String url, String message,
            XWalkJavascriptResult result) {
        final XWalkJavascriptResult fResult = result;
        AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(mContext);
        dialogBuilder.setTitle(mJSConfirmTitle)
                .setMessage(message)
                .setCancelable(true)
                .setPositiveButton(mOKButton, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        fResult.confirm();
                        dialog.dismiss();
                    }
                })
                .setNegativeButton(mCancelButton, null)
                .setOnCancelListener(new DialogInterface.OnCancelListener() {
                    @Override
                    public void onCancel(DialogInterface dialog) {
                        fResult.cancel();
                    }
                }).setOnKeyListener(new DialogInterface.OnKeyListener() {
                    @Override
                    public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                        if (keyCode == KeyEvent.KEYCODE_BACK) {
                            fResult.confirm();
                            return false;
                        } else {
                            return true;
                        }
                    }
                });
        mDialog = dialogBuilder.create();
        mDialog.show();
        return false;
    }

    private boolean onJsPrompt(XWalkView view, String url, String message,
            String defaultValue, XWalkJavascriptResult result) {
        final XWalkJavascriptResult fResult = result;
        AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(mContext);
        dialogBuilder.setTitle(mJSPromptTitle)
                .setMessage(message)
                .setPositiveButton(mOKButton, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        fResult.confirmWithResult(mPromptText.getText().toString());
                        dialog.dismiss();
                    }
                })
                .setNegativeButton(mCancelButton, null)
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

    @Override
    public void onFullscreenToggled(XWalkView view, boolean enterFullscreen) {
        Activity activity = view.getActivity();
        if (enterFullscreen) {
            if (VERSION.SDK_INT >= VERSION_CODES.KITKAT) {
                mSystemUiFlag = mDecorView.getSystemUiVisibility();
                mDecorView.setSystemUiVisibility(
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
                }
                if (!mOriginalFullscreen) {
                    activity.getWindow().setFlags(
                            WindowManager.LayoutParams.FLAG_FULLSCREEN,
                            WindowManager.LayoutParams.FLAG_FULLSCREEN);
                }
            }
        } else {
            if (VERSION.SDK_INT >= VERSION_CODES.KITKAT) {
                mDecorView.setSystemUiVisibility(mSystemUiFlag);
            } else {
                // Clear the activity fullscreen flag.
                if (!mOriginalFullscreen) {
                    activity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
                }
            }
        }
    }
}
