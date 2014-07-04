// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

import org.xwalk.core.XWalkNavigationItem;

/**
 * Test suite for GetItemAt().
 */
public class GetItemAtTest extends XWalkViewTestBase {
    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @SmallTest
    @Feature({"GetItemAt"})
    public void testGetItemAt() throws Throwable {
        final String url1 = "about:blank";
        final String url2 = "file:///android_asset/www/index.html";
        final String title ="Crosswalk Sample Application";
        assertTrue(getItemAtOnUiThread(-1) == null);
        assertTrue(getItemAtOnUiThread(java.lang.Integer.MAX_VALUE) == null);
        assertTrue(getItemAtOnUiThread(java.lang.Integer.MIN_VALUE) == null);
        loadUrlSync(url1);
        loadUrlSync(url2);
        assertTrue(getItemAtOnUiThread(historySizeOnUiThread()) == null);
        XWalkNavigationItem navigationItem1 = getItemAtOnUiThread(0);
        assertEquals(url1, navigationItem1.getUrl());
        XWalkNavigationItem navigationItem2 = getItemAtOnUiThread(1);
        assertEquals(url2, navigationItem2.getUrl());
        assertEquals(title, navigationItem2.getTitle());
    }
}
