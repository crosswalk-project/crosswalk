// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

/**
 * Test suite for CanGoForward().
 */
public class CanGoForwardTest extends XWalkViewInternalTestBase {
    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @SmallTest
    @Feature({"CanGoForward"})
    public void testCanGoForward() throws Throwable {
        final String url1 = "about:blank";
        final String url2 = "file:///android_asset/www/index.html";
        assertFalse(canGoForwardOnUiThread());
        loadUrlSync(url1);
        loadUrlSync(url2);
        assertFalse(canGoForwardOnUiThread());
        goBackSync();
        assertTrue(canGoForwardOnUiThread());
    }
}
