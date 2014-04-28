// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import org.xwalk.core.XWalkView;

import android.app.Activity;
import android.os.Bundle;
import android.widget.LinearLayout;

public class MultiXWalkViewActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.container);
        LinearLayout parent = (LinearLayout) findViewById(R.id.container);

        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.MATCH_PARENT);
        params.weight = 1;

        XWalkView view1 = new XWalkView(this, this);
        parent.addView(view1, params);

        XWalkView view2 = new XWalkView(this, this);
        parent.addView(view2, params);

        view1.load("http://www.intel.com", null);
        view2.load("http://www.baidu.com", null);
    }
}
