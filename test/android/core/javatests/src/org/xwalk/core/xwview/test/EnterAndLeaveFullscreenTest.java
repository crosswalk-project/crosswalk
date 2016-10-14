// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;


/**
 * Tests for the hasEnteredFullscreen() and leaveFullscreen() method.
 */
public class EnterAndLeaveFullscreenTest extends XWalkViewTestBase {
    private TestHelperBridge.OnFullscreenToggledHelper mOnFullscreenToggledHelper =
            mTestHelperBridge.getOnFullscreenToggledHelper();

    @Override
    protected void setUp() throws Exception {
        super.setUp();
    }

    @SmallTest
    @Feature({"XWalkView", "Fullscreen"})
    public void testEnterAndExitFullscreenWithAPI() throws Throwable {
        final String name = "fullscreen_enter_exit.html";
        String fileContent = getFileContent(name);
        int count = mOnFullscreenToggledHelper.getCallCount();

        loadDataSync(fileContent, "text/html", false);
        assertFalse(hasEnteredFullscreen());

        clickOnElementId("enter_fullscreen", null);
        mOnFullscreenToggledHelper.waitForCallback(count);
        assertTrue(hasEnteredFullscreen());

        count = mOnFullscreenToggledHelper.getCallCount();
        leaveFullscreen();
        mOnFullscreenToggledHelper.waitForCallback(count);
        assertFalse(hasEnteredFullscreen());
    }

    @SmallTest
    @Feature({"XWalkView", "Fullscreen"})
    public void testEnterAndExitFullscreenWithJS() throws Throwable {
        final String name = "fullscreen_enter_exit.html";
        String fileContent = getFileContent(name);
        int count = mOnFullscreenToggledHelper.getCallCount();

        loadDataSync(fileContent, "text/html", false);
        assertFalse(hasEnteredFullscreen());

        clickOnElementId("enter_fullscreen", null);
        mOnFullscreenToggledHelper.waitForCallback(count);

        count = mOnFullscreenToggledHelper.getCallCount();
        clickOnElementId("exit_fullscreen", null);
        mOnFullscreenToggledHelper.waitForCallback(count);
        assertFalse(hasEnteredFullscreen());
    }
}
