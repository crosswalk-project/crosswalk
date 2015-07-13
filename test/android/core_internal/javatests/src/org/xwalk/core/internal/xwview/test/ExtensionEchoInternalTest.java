// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;
import org.chromium.base.test.util.Feature;
import org.xwalk.core.internal.xwview.test.ExtensionEchoInternal;

/**
 * Test suite for ExtensionEchoInternal().
 */
public class ExtensionEchoInternalTest extends XWalkViewInternalTestBase {
    private final static String PASS_STRING = "Pass";

    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @SmallTest
    @Feature({"ExtensionEchoInternal"})
    public void testAsync() throws Throwable {
        ExtensionEchoInternal echo = new ExtensionEchoInternal();

        loadAssetFileAndWaitForTitle("echo_java.html");
        assertEquals(PASS_STRING, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"ExtensionEchoInternal"})
    public void testSync() throws Throwable {
        ExtensionEchoInternal echo = new ExtensionEchoInternal();

        loadAssetFile("echo_sync_java.html");
        assertEquals(PASS_STRING, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"ExtensionEchoInternal"})
    public void testMultiFrames() throws Throwable {
        ExtensionEchoInternal echo = new ExtensionEchoInternal();

        loadAssetFileAndWaitForTitle("framesEcho.html");
        assertEquals(PASS_STRING, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"ExtensionEchoInternal"})
    public void testBinary() throws Throwable {
        ExtensionEchoInternal echo = new ExtensionEchoInternal();

        loadAssetFileAndWaitForTitle("echo_binary_java.html");
        assertEquals(PASS_STRING, getTitleOnUiThread());
    }
}
