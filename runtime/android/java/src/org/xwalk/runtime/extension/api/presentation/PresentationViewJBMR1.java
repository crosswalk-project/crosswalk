// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension.api.presentation;

import android.app.Presentation;
import android.os.Build;
import android.content.Context;
import android.content.DialogInterface;
import android.view.Display;
import android.view.View;

/**
 * A wrapper class of android.app.Presentation class introduced from API level 17.
 */
public class PresentationViewJBMR1 extends PresentationView
        implements DialogInterface.OnShowListener, DialogInterface.OnDismissListener {

    private Presentation mPresentation;

    public PresentationViewJBMR1(Context context, Display display) {
        mPresentation = new Presentation(context, display);
    }

    @Override
    public void show() {
        mPresentation.show();
    }

    @Override
    public void dismiss() {
        mPresentation.dismiss();
    }

    @Override
    public void cancel() {
        mPresentation.cancel();
    }

    @Override
    public void setContentView(View contentView) {
        mPresentation.setContentView(contentView);
    }

    @Override
    public Display getDisplay() {
        return mPresentation.getDisplay();
    }

    @Override
    public void setPresentationListener(PresentationView.PresentationListener listener) {
        super.setPresentationListener(listener);

        if (mListener != null) {
            mPresentation.setOnShowListener(this);
            mPresentation.setOnDismissListener(this);
        } else {
            mPresentation.setOnShowListener(null);
            mPresentation.setOnDismissListener(null);
        }
    }

    @Override
    public void onShow(DialogInterface dialog) {
        if (mListener != null) mListener.onShow(this);
    }

    @Override
    public void onDismiss(DialogInterface dialog) {
        if (mListener != null) mListener.onDismiss(this);
    }
}

