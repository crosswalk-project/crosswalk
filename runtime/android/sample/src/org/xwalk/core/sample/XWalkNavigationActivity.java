// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import org.xwalk.core.XWalkView;
import org.xwalk.core.XWalkNavigationHistory;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageButton;

public class XWalkNavigationActivity extends Activity {

    private ImageButton mNextButton;
    private ImageButton mPrevButton;
    private XWalkView mXWalkView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.navigation_layout);
        mPrevButton = (ImageButton) findViewById(R.id.prev);
        mNextButton = (ImageButton) findViewById(R.id.next);
        mXWalkView = (XWalkView) findViewById(R.id.xwalkview);

        mPrevButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                // Go backward
                if (mXWalkView != null &&
                        mXWalkView.getNavigationHistory().canGoBack()) {
                    mXWalkView.getNavigationHistory().navigate(
                            XWalkNavigationHistory.Direction.BACKWARD, 1);
                }
            }
        });

        mNextButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                // Go forward
                if (mXWalkView != null &&
                        mXWalkView.getNavigationHistory().canGoForward()) {
                    mXWalkView.getNavigationHistory().navigate(
                            XWalkNavigationHistory.Direction.FORWARD, 1);
                }
            }
        });

        mXWalkView.load("http://www.baidu.com/", null);
    }
}
