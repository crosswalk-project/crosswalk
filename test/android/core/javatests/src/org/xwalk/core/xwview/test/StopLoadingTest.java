// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

/**
 * Test suite for stopLoading().
 */
public class StopLoadingTest extends XWalkViewTestBase {
    final String url = "file:///android_asset/www/index1.html";

    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @SmallTest
    @Feature({"stopLoading"})
    public void testStopLoadingWithStop() throws Throwable {
        stopLoadingSync(url);
        assertEquals("", getTitleOnUiThread());
    }
}
