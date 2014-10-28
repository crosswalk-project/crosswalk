// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

/**
 * Test suite for OnRequestFocus().
 */
public class OnRequestFocusTest extends XWalkViewTestBase {
    private TestHelperBridge.OnRequestFocusHelper mOnRequestFocusHelper;

    @Override
    public void setUp() throws Exception {
        super.setUp();
        mOnRequestFocusHelper = mTestHelperBridge.getOnRequestFocusHelper();
    }

    @SmallTest
    @Feature({"OnRequestFocus"})
    public void testOnRequestFocus() throws Throwable {
        final String url = "file:///android_asset/www/request_focus_main.html";
        int count = mOnRequestFocusHelper.getCallCount();

        loadUrlSync(url);
        clickOnElementId("left_frame", "LeftFrame");
        mOnRequestFocusHelper.waitForCallback(count);
        assertTrue(mOnRequestFocusHelper.getCalled());
    }
}
