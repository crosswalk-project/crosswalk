// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import org.xwalk.core.XWalkView;

import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.LinearLayout;

public class MultiXWalkViewOverlayActivity extends XWalkBaseActivity {

    private XWalkView mXWalkView2;
    private Button mSwapButton;

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

        mSwapButton = new Button(this);
        mSwapButton.setText("Swap XWalkView");
        parent.addView(mSwapButton, buttonParams);
        mSwapButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                if (mXWalkView.getVisibility() == View.VISIBLE) {
                    mXWalkView.setVisibility(View.INVISIBLE);
                    mXWalkView2.setVisibility(View.VISIBLE);
                } else {
                    mXWalkView.setVisibility(View.VISIBLE);
                    mXWalkView2.setVisibility(View.INVISIBLE);
                }
            }
        });
        mXWalkView = new XWalkView(this, this);
        parent.addView(mXWalkView, params);
        mXWalkView.setVisibility(View.VISIBLE);

        mXWalkView2 = new XWalkView(this, this);
        parent.addView(mXWalkView2, params);
        mXWalkView2.setVisibility(View.INVISIBLE);

        mXWalkView.loadUrl("http://www.intel.com");
        mXWalkView2.loadUrl("http://www.baidu.com");
    }
}
