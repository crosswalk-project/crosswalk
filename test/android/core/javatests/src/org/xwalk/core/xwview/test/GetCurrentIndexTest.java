// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

/**
 * Test suite for GetCurrentIndex().
 */
public class GetCurrentIndexTest extends XWalkViewTestBase {
    @Override
    public void setUp() throws Exception {
        super.setUp();

        setXWalkClient(new XWalkViewTestBase.TestXWalkClient());
    }

    @SmallTest
    @Feature({"GetCurrentIndex"})
    public void testGetCurrentIndex() throws Throwable {
        final String url1 = "about:blank";
        final String url2 = "file:///android_asset/www/index.html";
        loadUrlSync(url1);
        loadUrlSync(url2);
        goBackSync();
        assertEquals(0, getCurrentIndexOnUiThread());
        goForwardSync();
        assertEquals(1, getCurrentIndexOnUiThread());
    }
}
