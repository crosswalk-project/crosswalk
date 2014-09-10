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

    @Override
    protected void setUp() throws Exception {
        super.setUp();
    }

    @SmallTest
    @Feature({"XWalkView", "Fullscreen"})
    public void testEnterAndExitFullscreen() throws Throwable {
        final String name = "fullscreen_enter_exit.html";
        String fileContent = getFileContent(name);

        loadDataSync(name, fileContent, "text/html", false);

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                try {
                    clickOnElementId("enter_fullscreen", null);
                    assertTrue(getXWalkView().hasEnteredFullscreen());
                    getXWalkView().leaveFullscreen();
                    assertFalse(getXWalkView().hasEnteredFullscreen());

                    clickOnElementId("enter_fullscreen", null);
                    clickOnElementId("exit_fullscreen", null);
                    assertFalse(getXWalkView().hasEnteredFullscreen());
                } catch (Throwable e) {
                    e.printStackTrace();
                }
            }
        });
    }
}
