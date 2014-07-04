// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

import org.xwalk.core.XWalkNavigationItem;

/**
 * Test suite for GetCurrentItem().
 */
public class GetCurrentItemTest extends XWalkViewTestBase {
    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @SmallTest
    @Feature({"GetCurrentItem"})
    public void testGetCurrentItem() throws Throwable {
        final String expectedUrl = "file:///android_asset/www/index.html";
        final String expectedTitle ="Crosswalk Sample Application";
        loadUrlSync(expectedUrl);
        XWalkNavigationItem navigationItem = getCurrentItemOnUiThread();
        assertEquals(expectedUrl, navigationItem.getUrl());
        assertEquals(expectedTitle, navigationItem.getTitle());
    }
}
