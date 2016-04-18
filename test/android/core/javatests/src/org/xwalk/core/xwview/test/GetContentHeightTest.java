// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;

import java.io.InputStream;
import java.net.URL;
import java.util.concurrent.Callable;

import org.chromium.base.test.util.Feature;
import org.chromium.net.test.util.TestWebServer;

import org.xwalk.core.XWalkUIClient;
import org.xwalk.core.XWalkView;
import org.xwalk.core.xwview.test.util.CommonResources;

/**
 * Test suite for getFavicon().
 */
public class GetContentHeightTest extends XWalkViewTestBase {
    private XWalkView mXWalkView;

    @Override
    public void setUp() throws Exception {
        super.setUp();

        mXWalkView = getXWalkView();
    }

    @SmallTest
    @Feature({"getContentHeight"})
    public void testGetContentHeight() throws Throwable {
        final String expectedUrl = "file:///android_asset/www/index.html";
        loadUrlAsync(expectedUrl);
        pollOnUiThread(new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return mXWalkView.getContentHeight() != 0;
            }
        });
        //loadUrlSync(expectedUrl);
        int height = getContentHeightOnUiThread();
        assertTrue(height != 0);
    }
}

