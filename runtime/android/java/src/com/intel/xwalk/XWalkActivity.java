// Copyright (c) 2012 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package com.intel.xwalk;

import android.app.Activity;
import android.os.Bundle;
import android.webkit.WebView;

public class XWalkActivity extends Activity {
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        XWalkView webView = (XWalkView) findViewById(R.id.xwalkView);
        webView.loadUrl("http://www.google.com");
    }
}

