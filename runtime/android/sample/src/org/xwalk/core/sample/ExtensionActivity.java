// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import org.xwalk.core.sample.ExtensionEcho;
import org.xwalk.core.XWalkView;

import android.os.Bundle;

public class ExtensionActivity extends XWalkBaseActivity {
    private ExtensionEcho mExtension;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.xwview_layout);
        mXWalkView = (XWalkView) findViewById(R.id.xwalkview);
        mExtension = new ExtensionEcho();

        mXWalkView.loadUrl("file:///android_asset/echo_java.html");
    }
}
