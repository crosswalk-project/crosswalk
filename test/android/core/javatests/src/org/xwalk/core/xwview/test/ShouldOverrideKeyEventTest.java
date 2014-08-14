// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;
import android.view.KeyEvent;

import org.chromium.base.test.util.Feature;

/**
 * Test suite for shouldOverrideKeyEvent().
 */
public class ShouldOverrideKeyEventTest extends XWalkViewTestBase {
    TestHelperBridge.OverrideOrUnhandledKeyEventHelper mOverrideOrUnhandledKeyEventHelper;

    @Override
    public void setUp() throws Exception {
        super.setUp();
        mOverrideOrUnhandledKeyEventHelper = mTestHelperBridge.getOverrideOrUnhandledKeyEventHelper();
    }

    @SmallTest
    @Feature({"ShouldOverrideKeyEvent"})
    public void testShouldOverrideKeyEvent() throws Throwable {
        final String name = "index.html";
        String fileContent = getFileContent(name);
        int count = mOverrideOrUnhandledKeyEventHelper.getCallCount();

        loadDataAsync(null, fileContent, "text/html", false);
        simulateKeyAction(KeyEvent.ACTION_DOWN);
        mOverrideOrUnhandledKeyEventHelper.waitForCallback(count);

        KeyEvent event = mOverrideOrUnhandledKeyEventHelper.getKeyEvent();
        assertTrue(KeyEvent.ACTION_DOWN == event.getAction());
    }
}
