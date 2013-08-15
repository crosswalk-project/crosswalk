// Copyright (c) 2013 Intel Corporation. All rights reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.client;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.pm.ActivityInfo;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Message;
import android.view.View;
import android.webkit.WebStorage;
import android.webkit.GeolocationPermissions;
import android.webkit.ConsoleMessage;
import android.webkit.ValueCallback;
import android.widget.EditText;

import org.xwalk.core.JsPromptResult;
import org.xwalk.core.JsResult;
import org.xwalk.core.XWalkView;
import org.xwalk.core.XWalkWebChromeClient;

public class XWalkDefaultWebChromeClient extends XWalkWebChromeClient {
    private Context mContext;
    private AlertDialog mDialog;
    private EditText mPromptText;

    public XWalkDefaultWebChromeClient(Context context) {
        mContext = context;
    }

    @Override
    public boolean onJsAlert(XWalkView view, String url, String message,
            final JsResult result) {
        AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(mContext);
        // TODO(shouqun): the strings need to be localized.
        dialogBuilder.setTitle("JavaScript Alert")
                .setMessage(message)
                .setCancelable(false)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        result.confirm();
                        dialog.dismiss();
                    }
                });
        mDialog = dialogBuilder.create();
        mDialog.show();
        return false;
    }

    @Override
    public boolean onJsConfirm(XWalkView view, String url, String message,
            final JsResult result) {
        AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(mContext);
        // TODO(shouqun): the strings need to be localized.
        dialogBuilder.setTitle("JavaScript Confirm")
                .setMessage(message)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        result.confirm();
                        dialog.dismiss();
                    }
                })
                .setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        result.cancel();
                        dialog.dismiss();
                    }
                });
        mDialog = dialogBuilder.create();
        mDialog.show();
        return false;
    }

    @Override
    public boolean onJsPrompt(XWalkView view, String url, String message,
            String defaultValue, final JsPromptResult result) {
        AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(mContext);
        // TODO(shouqun): the strings need to be localized.
        dialogBuilder.setTitle("JavaScript Prompt")
                .setMessage(message)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        result.confirm(mPromptText.getText().toString());
                        dialog.dismiss();
                    }
                })
                .setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        result.cancel();
                        dialog.dismiss();
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
}
