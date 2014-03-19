// Copyright (c) 2014 Intel Corporation. All rights reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.view.View;

/**
 * It's the default implementation class for XWalkResourceClient.
 */
public class XWalkResourceClientImpl extends XWalkResourceClient {
    private Context mContext;
    private AlertDialog mDialog;
    private XWalkView mXWalkView;

    public XWalkResourceClientImpl(Context context, XWalkView view) {
        mContext = context;
        mXWalkView = view;
    }

    @Override
    public void onReceivedLoadError(XWalkView view, int errorCode,
            String description, String failingUrl) {
        AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(mContext);
        dialogBuilder.setTitle(android.R.string.dialog_alert_title)
                .setMessage(description)
                .setCancelable(false)
                .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                    }
                });
        mDialog = dialogBuilder.create();
        mDialog.show();
    }
}
