// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

import org.xwalk.core.XWalkNavigationItem;

/**
 * Test suite for HasItemAt().
 */
public class HasItemAtTest extends XWalkViewTestBase {
    @Override
    public void setUp() throws Exception {
        super.setUp();

        setXWalkClient(new XWalkViewTestBase.TestXWalkClient());
    }

    @SmallTest
    @Feature({"HasItemAt"})
    public void testHasItemAt() throws Throwable {
        final String url1 = "about:blank";
        final String url2 = "file:///android_asset/www/index.html";
        assertFalse(hasItemAtOnUiThread(-1));
        assertFalse(hasItemAtOnUiThread(java.lang.Integer.MAX_VALUE));
        assertFalse(hasItemAtOnUiThread(java.lang.Integer.MIN_VALUE));
        assertFalse(hasItemAtOnUiThread(0));
        loadUrlSync(url1);
        assertTrue(hasItemAtOnUiThread(0));
        assertFalse(hasItemAtOnUiThread(1));
        loadUrlSync(url2);
        assertTrue(hasItemAtOnUiThread(1));
    }
}
