// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;
import android.view.KeyEvent;

import org.chromium.base.test.util.Feature;
import org.xwalk.core.XWalkUIClient;
import org.xwalk.core.XWalkView;

/**
 * Test suite for onUnhandledKeyEvent().
 */
public class OnUnhandledKeyEventTest extends XWalkViewTestBase {
    TestHelperBridge.OverrideOrUnhandledKeyEventHelper mOverrideOrUnhandledKeyEventHelper;

    class TestXWalkUIClientForKeyEvent extends XWalkUIClient {
        public TestXWalkUIClientForKeyEvent() {
            super(getXWalkView());
        }

        @Override
        public boolean shouldOverrideKeyEvent(XWalkView view, KeyEvent event) {
            return false;
        }

        @Override
        public void onUnhandledKeyEvent(XWalkView view, KeyEvent event) {
            mTestHelperBridge.overrideOrUnhandledKeyEvent(event);
        }
    }

    @Override
    public void setUp() throws Exception {
        super.setUp();
        mOverrideOrUnhandledKeyEventHelper = mTestHelperBridge.getOverrideOrUnhandledKeyEventHelper();
        setUIClient(new TestXWalkUIClientForKeyEvent());
    }

    @SmallTest
    @Feature({"onUnhandledKeyEvent"})
    public void testOnUnhandledKeyEvent() throws Throwable {
        final String name = "index.html";
        String fileContent = getFileContent(name);
        int count = mOverrideOrUnhandledKeyEventHelper.getCallCount();
        loadDataAsync(null, fileContent, "text/html", false);
        simulateKeyAction(KeyEvent.ACTION_UP);
        mOverrideOrUnhandledKeyEventHelper.waitForCallback(count);

        KeyEvent event = mOverrideOrUnhandledKeyEventHelper.getKeyEvent();
        assertTrue(KeyEvent.ACTION_UP == event.getAction());
    }
}
