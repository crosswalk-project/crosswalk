// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import org.xwalk.core.XWalkPreferences;
import org.xwalk.core.XWalkView;

import android.app.Activity;
import android.os.Bundle;

public class XWalkPreferencesActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.xwview_layout);
        XWalkView xwalkView = (XWalkView) findViewById(R.id.xwalkview);

        // Enable remote debugging.
        // You can debug the web content via PC chrome.
        XWalkPreferences.setValue(XWalkPreferences.REMOTE_DEBUGGING, true);

        xwalkView.load("http://www.baidu.com/", null);
    }
}
