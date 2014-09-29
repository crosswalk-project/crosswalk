// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

/**
 * Test suite for OnFullscreenToggled().
 */
public class OnFullscreenToggledTest extends XWalkViewTestBase {
    private TestHelperBridge.OnFullscreenToggledHelper mOnFullscreenToggledHelper;

    @Override
    public void setUp() throws Exception {
        super.setUp();
        mOnFullscreenToggledHelper = mTestHelperBridge.getOnFullscreenToggledHelper();
    }

    @SmallTest
    @Feature({"OnFullscreenToggled"})
    public void testOnFullscreenToggled() throws Throwable {
        final String name = "fullscreen_togged.html";
        String fileContent = getFileContent(name);
        int count = mOnFullscreenToggledHelper.getCallCount();

        loadDataSync(null, fileContent, "text/html", false);
        clickOnElementId("fullscreen_toggled", null);
        mOnFullscreenToggledHelper.waitForCallback(count);
        assertTrue(mOnFullscreenToggledHelper.getEnterFullscreen());
    }
}
