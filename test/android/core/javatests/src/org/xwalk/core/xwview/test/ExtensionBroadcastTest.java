// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.graphics.Bitmap;
import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;

import org.chromium.base.test.util.Feature;

import org.xwalk.core.XWalkView;
import org.xwalk.core.internal.XWalkClient;
import org.xwalk.core.internal.XWalkWebChromeClient;
import org.xwalk.core.xwview.test.ExtensionBroadcast;

/**
 * Test suite for ExtensionBroadcast().
 */
public class ExtensionBroadcastTest extends XWalkViewTestBase {
    @Override
    public void setUp() throws Exception {
        super.setUp();

        setXWalkClient(new XWalkViewTestBase.TestXWalkClient());
        setXWalkWebChromeClient(new XWalkViewTestBase.TestXWalkWebChromeClient());
    }

    @SmallTest
    @Feature({"ExtensionBroadcast"})
    public void testExtensionBroadcast() throws Throwable {
        ExtensionBroadcast broadcast = new ExtensionBroadcast();

        loadAssetFileAndWaitForTitle("broadcast.html");
        assertEquals("Pass", getTitleOnUiThread());
    }
}
