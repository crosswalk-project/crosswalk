// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

import org.xwalk.core.XWalkNavigationItem;

/**
 * Test suite for HistorySize().
 */
public class HistorySizeTest extends XWalkViewTestBase {
    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @SmallTest
    @Feature({"HistorySize"})
    public void testHistorySize() throws Throwable {
        final String url1 = "about:blank";
        final String url2 = "file:///android_asset/www/index.html";
        assertEquals(0, historySizeOnUiThread());
        loadUrlSync(url1);
        assertEquals(1, historySizeOnUiThread());
        loadUrlSync(url2);
        assertEquals(2, historySizeOnUiThread());
        goBackSync();
        assertEquals(2, historySizeOnUiThread());
    }
}
