// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import org.xwalk.core.XWalkView;

import android.app.Activity;

public class XWalkBaseActivity extends Activity {
    protected XWalkView mXWalkView;

    /*
     * When the activity is paused, XWalkView.onHide() and XWalkView.pauseTimers() need to be called.
     */
    @Override
    public void onPause() {
        super.onPause();
        if (mXWalkView != null) {
            mXWalkView.onHide();
            mXWalkView.pauseTimers();
        }
    }

    /*
     * When the activity is resumed, XWalkView.onShow() and XWalkView.resumeTimers() need to be called.
     */
    @Override
    public void onResume() {
        super.onResume();
        if (mXWalkView != null) {
            mXWalkView.onShow();
            mXWalkView.resumeTimers();
        }
    }

    /*
     * Call onDestroy on XWalkView to release native resources when the activity is destroyed.
     */
    @Override
    public void onDestroy() {
        super.onDestroy();
        if (mXWalkView != null) {
            mXWalkView.onDestroy();
        }
    }
}
