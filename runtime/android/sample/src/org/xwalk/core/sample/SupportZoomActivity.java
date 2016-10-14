// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import org.xwalk.core.XWalkView;
import org.xwalk.core.XWalkSettings;

import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.LinearLayout;

public class SupportZoomActivity extends XWalkBaseActivity {
    private Button mEnableBuiltInZoomButton;
    private Button mDisableBuiltInZoomButton;
    private Button mEnableDoubleTapZoomButton;
    private Button mDisableDoubleTapZoomButton;
    private XWalkSettings mXWalkSettings;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.container);
        LinearLayout parent = (LinearLayout) findViewById(R.id.container);

        FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.FILL_PARENT,
                FrameLayout.LayoutParams.FILL_PARENT);
        FrameLayout.LayoutParams buttonParams = new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.FILL_PARENT,
                FrameLayout.LayoutParams.WRAP_CONTENT);

        mEnableBuiltInZoomButton = new Button(this);
        mEnableBuiltInZoomButton.setText("Enable BuiltInZoom");
        parent.addView(mEnableBuiltInZoomButton, buttonParams);
        mEnableBuiltInZoomButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                setAndLoadForBuiltInZoom(true);
            }
        });

        mEnableDoubleTapZoomButton = new Button(this);
        mEnableDoubleTapZoomButton.setText("Disable BuiltInZoom");
        parent.addView(mEnableDoubleTapZoomButton, buttonParams);
        mEnableDoubleTapZoomButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                setAndLoadForBuiltInZoom(false);
            }
        });

        mEnableDoubleTapZoomButton = new Button(this);
        mEnableDoubleTapZoomButton.setText("Enable DoubleTapZoom");
        parent.addView(mEnableDoubleTapZoomButton, buttonParams);
        mEnableDoubleTapZoomButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                setAndLoadForDoubleTapZoom(true);
            }
        });

        mDisableDoubleTapZoomButton = new Button(this);
        mDisableDoubleTapZoomButton.setText("Disable DoubleTapZoom");
        parent.addView(mDisableDoubleTapZoomButton, buttonParams);
        mDisableDoubleTapZoomButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                setAndLoadForDoubleTapZoom(false);;
            }
        });
        mXWalkView = new XWalkView(this, this);
        parent.addView(mXWalkView, params);
        mXWalkSettings = mXWalkView.getSettings();
    }

    void setAndLoadForBuiltInZoom(boolean flag) {
        mXWalkSettings.setBuiltInZoomControls(flag);
        mXWalkView.loadUrl("file:///android_asset/builtinzoom.html");
    }

    void setAndLoadForDoubleTapZoom(boolean flag) {
        // setUseWideViewPort() should be called at first.
        mXWalkSettings.setUseWideViewPort(flag);
        mXWalkSettings.setBuiltInZoomControls(flag);
        mXWalkView.loadUrl("file:///android_asset/doubletapzoom.html");
    }
}
