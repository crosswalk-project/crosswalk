// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import org.xwalk.core.XWalkView;

import android.os.Bundle;

public class OnHideOnShowActivity extends XWalkBaseActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.xwview_layout);
        mXWalkView = (XWalkView) findViewById(R.id.xwalkview);

        // The web page below will display a video.
        // When home button is pressed, the activity will be in background, and the video will be paused.
        mXWalkView.load("http://www.w3.org/2010/05/video/mediaevents.html", null);
    }
}
