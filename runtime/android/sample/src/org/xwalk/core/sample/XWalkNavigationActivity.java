// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import org.xwalk.core.XWalkNavigationItem;
import org.xwalk.core.XWalkView;
import org.xwalk.core.XWalkNavigationHistory;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageButton;
import android.widget.TextView;

public class XWalkNavigationActivity extends Activity {

    private ImageButton mNextButton;
    private ImageButton mPrevButton;
    private XWalkView mXWalkView;
    String url, originalUrl, title;
    TextView text1, text2, text3;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.navigation_layout);
        mPrevButton = (ImageButton) findViewById(R.id.prev);
        mNextButton = (ImageButton) findViewById(R.id.next);
        mXWalkView = (XWalkView) findViewById(R.id.xwalkview);

        text1 = (TextView) super.findViewById(R.id.text1);
        text2 = (TextView) super.findViewById(R.id.text2);
        text3 = (TextView) super.findViewById(R.id.text3);

        mPrevButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                // Go backward
                if (mXWalkView != null &&
                        mXWalkView.getNavigationHistory().canGoBack()) {
                    mXWalkView.getNavigationHistory().navigate(
                            XWalkNavigationHistory.Direction.BACKWARD, 1);
                }
                XWalkNavigationItem navigationItem = mXWalkView.getNavigationHistory().getCurrentItem();
                url = navigationItem.getUrl();
                originalUrl = navigationItem.getOriginalUrl();
                title = navigationItem.getTitle();

                text1.setText(title);
                text2.setText(url);
                text3.setText(originalUrl);
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
                XWalkNavigationItem navigationItem = mXWalkView.getNavigationHistory().getCurrentItem();
                url = navigationItem.getUrl();
                originalUrl = navigationItem.getOriginalUrl();
                title = navigationItem.getTitle();

                text1.setText(title);
                text2.setText(url);
                text3.setText(originalUrl);
            }
        });

        mXWalkView.load("http://www.baidu.com/", null);
    }
}
