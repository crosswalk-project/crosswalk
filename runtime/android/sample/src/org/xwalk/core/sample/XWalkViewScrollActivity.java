// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import org.xwalk.core.XWalkResourceClient;

import org.xwalk.core.XWalkView;

import android.os.Bundle;

import android.util.Log;

public class XWalkViewScrollActivity extends XWalkBaseActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.xwview_layout);
        mXWalkView = (XWalkView) findViewById(R.id.xwalkview);

        mXWalkView.load("https://crosswalk-project.org/", null);
        mXWalkView.setResourceClient(new XWalkResourceClient(mXWalkView) {
            @Override
            public void onLoadFinished(XWalkView view, String url) {
                super.onLoadFinished(view, url);
                view.overScrollBy(0, 3000,
                                  view.computeHorizontalScrollOffset(),
                                  view.computeVerticalScrollOffset(),
                                  view.computeHorizontalScrollRange(),
                                  view.computeVerticalScrollRange(),
                                  0, 0, true);
                Log.i("XWALK", "horizontal range = " + view.computeHorizontalScrollRange());
                Log.i("XWALK", "vertical range = " + view.computeVerticalScrollRange());
                Log.i("XWALK", "horizontal offset = " + view.computeHorizontalScrollOffset());
                Log.i("XWALK", "vertical offset = " + view.computeVerticalScrollOffset());
                Log.i("XWALK", "vertical extent = " + view.computeVerticalScrollExtent());
            }
        });
    }
}
