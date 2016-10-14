// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

/**
 * Test suite for OnScaleChanged().
 */
public class OnScaleChangedTest extends XWalkViewTestBase {
    private TestHelperBridge.OnScaleChangedHelper mOnScaleChangedHelper;

    @Override
    public void setUp() throws Exception {
        super.setUp();
        mOnScaleChangedHelper = mTestHelperBridge.getOnScaleChangedHelper();
    }

    @SmallTest
    @Feature({"OnScaleChanged"})
    public void testOnScaleChanged() throws Throwable {
        final String name = "scale_changed.html";
        String fileContent = getFileContent(name);
        int count = mOnScaleChangedHelper.getCallCount();

        loadDataAsync(fileContent, "text/html", false);
        mOnScaleChangedHelper.waitForCallback(count);
        assertTrue(Float.compare(mOnScaleChangedHelper.getNewScale(), 0.0f) > 0);
    }
}
