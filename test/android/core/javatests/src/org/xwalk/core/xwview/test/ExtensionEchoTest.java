// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;
import org.chromium.base.test.util.Feature;
import org.xwalk.core.xwview.test.ExtensionEcho;

/**
 * Test suite for ExtensionEcho().
 */
public class ExtensionEchoTest extends XWalkViewTestBase {
    private final static String PASS_STRING = "Pass";

    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @SmallTest
    @Feature({"ExtensionEcho"})
    public void testAsync() throws Throwable {
        ExtensionEcho echo = new ExtensionEcho();

        loadAssetFileAndWaitForTitle("echo_java.html");
        assertEquals(PASS_STRING, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"ExtensionEcho"})
    public void testSync() throws Throwable {
        ExtensionEcho echo = new ExtensionEcho();

        loadAssetFile("echo_sync_java.html");
        assertEquals(PASS_STRING, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"ExtensionEcho"})
    public void testMultiFrames() throws Throwable {
        ExtensionEcho echo = new ExtensionEcho();

        loadAssetFileAndWaitForTitle("framesEcho.html");
        assertEquals(PASS_STRING, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"ExtensionEcho"})
    public void testBinary() throws Throwable {
        ExtensionEcho echo = new ExtensionEcho();

        loadAssetFileAndWaitForTitle("echo_binary_java.html");
        assertEquals(PASS_STRING, getTitleOnUiThread());
    }
}
