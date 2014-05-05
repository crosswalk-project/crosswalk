// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import org.xwalk.core.XWalkView;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

public class XWalkVersionAndAPIVersion extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.version_layout);
        XWalkView xwalkView = (XWalkView) findViewById(R.id.xwalkview);
        String apiVersion = xwalkView.getAPIVersion();
        String xwalkVersion = xwalkView.getXWalkVersion();
        TextView text1 = (TextView) super.findViewById(R.id.text1);
        text1.setText("API Version: " +apiVersion + "; XWalk Version: " + xwalkVersion);
        xwalkView.load("http://www.google.com/", "");
    }
}
