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
import org.xwalk.core.xwview.test.ExtensionEcho;

/**
 * Test suite for ExtensionEcho().
 */
public class ExtensionEchoTest extends XWalkViewTestBase {
    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @SmallTest
    @Feature({"ExtensionEcho"})
    public void testExtensionEcho() throws Throwable {
        ExtensionEcho echo = new ExtensionEcho();

        loadAssetFileAndWaitForTitle("echo.html");
        assertEquals("Pass", getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"ExtensionEcho"})
    public void testExtensionEchoSync() throws Throwable {
        ExtensionEcho echo = new ExtensionEcho();

        loadAssetFile("echoSync.html");
        assertEquals("Pass", getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"ExtensionEcho"})
    public void testExtensionEchoMultiFrames() throws Throwable {
        ExtensionEcho echo = new ExtensionEcho();

        loadAssetFileAndWaitForTitle("framesEcho.html");
        assertEquals("Pass", getTitleOnUiThread());
    }
}
