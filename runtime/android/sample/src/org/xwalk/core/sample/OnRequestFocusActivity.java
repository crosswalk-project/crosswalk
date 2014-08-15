// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import org.xwalk.core.XWalkView;

import android.os.Bundle;
import android.widget.TextView;

public class OnRequestFocusActivity extends XWalkBaseActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.version_layout);
        mXWalkView = (XWalkView) findViewById(R.id.xwalkview);
        TextView textView = (TextView) super.findViewById(R.id.text1);
        textView.setText("Will display the test result.");
        mXWalkView.setUIClient(new XWalkBaseActivity.UIClientBase(mXWalkView, textView, TestFrom.FOCUS));
        mXWalkView.load("file:///android_asset/request_focus_main.html", null);
    }
}
