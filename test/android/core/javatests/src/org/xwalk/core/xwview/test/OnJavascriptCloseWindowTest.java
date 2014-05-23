// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

/**
 * Test suite for onJavascriptCloseWindow().
 */
public class OnJavascriptCloseWindowTest extends XWalkViewTestBase {
    TestHelperBridge.OnJavascriptCloseWindowHelper mCloseWindowHelper;

    @Override
    public void setUp() throws Exception {
        super.setUp();

        mCloseWindowHelper = mTestHelperBridge.getOnJavascriptCloseWindowHelper();
    }

    @SmallTest
    @Feature({"OnJavascriptCloseWindow"})
    public void testOnJavascriptCloseWindow() throws Throwable {
        final String url = "window.close.html";
        int count = mCloseWindowHelper.getCallCount();

        loadAssetFile(url);
        mCloseWindowHelper.waitForCallback(count);
        assertTrue(mCloseWindowHelper.getCalled());
    }
}
