// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import org.xwalk.core.XWalkView;

import android.app.Activity;
import android.os.Bundle;

public class OnHideOnShowActivity extends Activity {

    private XWalkView mXWalkView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.xwview_layout);
        mXWalkView = (XWalkView) findViewById(R.id.xwalkview);

        mXWalkView.load("http://www.w3.org/2010/05/video/mediaevents.html", null);
    }

    @Override
    public void onPause() {
        super.onPause();
        // It will pause the video, when the app in background.
        mXWalkView.onHide();
    }

    @Override
    public void onResume() {
        super.onResume();
        // Need to call onShow() when onHide() was called.
        mXWalkView.onShow();
    }
}
