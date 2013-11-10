// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.graphics.Bitmap;
import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;

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

        class TestXWalkChromeClient extends XWalkWebChromeClient {
            @Override
            public void onReceivedTitle(XWalkView view, String title) {
                mTestContentsClient.onTitleChanged(title);
            }
        }
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getXWalkView().setXWalkClient(new TestXWalkClient());
                getXWalkView().setXWalkWebChromeClient(new TestXWalkChromeClient());
            }
        });
    }

    @SmallTest
    @Feature({"ExtensionBroadcast"})
    public void testExtensionBroadcast() throws Throwable {
        ExtensionBroadcast broadcast = new ExtensionBroadcast();

        loadAssetFileAndWaitForTitle("broadcast.html");
        assertEquals("Pass", getTitleOnUiThread());
    }
}
