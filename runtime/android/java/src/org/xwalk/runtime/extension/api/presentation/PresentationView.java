// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension.api.presentation;

import android.os.Build;
import android.content.Context;
import android.view.Display;
import android.view.View;

/**
 * A helper class to abstract the presentation view for different android build version.
 *
 * A PresentationView is a special kind of UI widget whose purpose is to present content
 * on a secondary display. A PresentationView is associated with the target Display at
 * creation time and configures its context and resource configuration according to the
 * display's metrics.
 */
public abstract class PresentationView {
    protected PresentationListener mListener;

    /**
     * Return an instance of PresentationView according to the build version.
     */
    public static PresentationView createInstance(Context context, Display display) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
            return new PresentationViewJBMR1(context, display);
        } else {
            return new PresentationViewNull();
        }
    }

    public abstract void show();

    public abstract void dismiss();

    public abstract void cancel();

    public abstract void setContentView(View contentView);

    public abstract Display getDisplay();

    public void setPresentationListener(PresentationListener listener) {
        mListener = listener;
    }

    /**
     * Interface used to allow the creator of a PresentationView to run some code
     * when it is showed or dismissed.
     */
    public interface PresentationListener {
        /**
         * Invoked when the presentation view is showed.
         */
        public void onShow(PresentationView view);

        /**
         * Invoked when the presentation view is dismissed.
         */
        public void onDismiss(PresentationView view);
    }
}

