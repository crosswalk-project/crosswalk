// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.view.WindowManager;
import android.widget.LinearLayout;

import org.xwalk.core.XWalkActivityDelegate;

/*
 * This is a lightweight activity for tests that only require XWalk functionality.
 */
public class XWalkViewTestRunnerActivity extends Activity {

    private LinearLayout mLinearLayout;
    private XWalkActivityDelegate mActivityDelegate;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Runnable cancelCommand = new Runnable() {
            @Override
            public void run() {
                finish();
            }
        };
        Runnable completeCommand = new Runnable() {
            @Override
            public void run() {
            }
        };
        mActivityDelegate = new XWalkActivityDelegate(this, cancelCommand, completeCommand);

        getWindow().setFlags(
                WindowManager.LayoutParams.FLAG_HARDWARE_ACCELERATED,
                WindowManager.LayoutParams.FLAG_HARDWARE_ACCELERATED);

        mLinearLayout = new LinearLayout(this);
        mLinearLayout.setOrientation(LinearLayout.VERTICAL);
        mLinearLayout.setShowDividers(LinearLayout.SHOW_DIVIDER_MIDDLE);
        mLinearLayout.setLayoutParams(new LayoutParams(LayoutParams.WRAP_CONTENT,
                LayoutParams.WRAP_CONTENT));

        setContentView(mLinearLayout);
    }

    public boolean isXWalkReady() {
        return mActivityDelegate.isXWalkReady();
    }

    /**
     * Adds a view to the main linear layout.
     */
    public void addView(View view) {
        view.setLayoutParams(new LinearLayout.LayoutParams(
                LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT, 1f));
        mLinearLayout.addView(view);
    }

    /**
     * Clears the main linear layout.
     */
    public void removeAllViews() {
        mLinearLayout.removeAllViews();
    }
}
