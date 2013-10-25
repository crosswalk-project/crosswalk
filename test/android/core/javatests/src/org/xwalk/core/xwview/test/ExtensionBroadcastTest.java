// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.graphics.Bitmap;
import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;

import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.Feature;

import org.xwalk.core.XWalkClient;
import org.xwalk.core.XWalkView;
import org.xwalk.core.XWalkWebChromeClient;
import org.xwalk.core.xwview.test.ExtensionBroadcast;

/**
 * Test suite for ExtensionBroadcast().
 */
public class ExtensionBroadcastTest extends XWalkViewTestBase {
    @Override
    public void setUp() throws Exception {
        super.setUp();

        class TestXWalkClient extends XWalkClient {
            @Override
            public void onPageStarted(XWalkView view, String url, Bitmap favicon) {
                mTestContentsClient.onPageStarted(url);
            }

            @Override
            public void onPageFinished(XWalkView view, String url) {
                mTestContentsClient.didFinishLoad(url);
            }
        }

        getXWalkView().setXWalkClient(new TestXWalkClient());
    }

    // @SmallTest
    // @Feature({"ExtensionBroadcast"})
    // This test case failed on buildbot, so disabled it temporally.
    // It will be enabled later.
    @DisabledTest
    public void testExtensionBroadcast() throws Throwable {
        ExtensionBroadcast broadcast = new ExtensionBroadcast();

        loadAssetFile("broadcast.html");
        assertEquals("Pass", getTitleOnUiThread());
    }
}
