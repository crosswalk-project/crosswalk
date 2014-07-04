// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.graphics.Bitmap;
import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;
import org.chromium.base.test.util.Feature;
import org.xwalk.core.XWalkView;
import org.xwalk.core.internal.XWalkClient;
import org.xwalk.core.internal.XWalkWebChromeClient;

/**
 * Test suite for onUpdateTitle().
 */
public class OnUpdateTitleTest extends XWalkViewTestBase {
    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @SmallTest
    @Feature({"OnUpdateTitle"})
    public void testOnUpdateTitle() throws Throwable {
        final String name = "index.html";
        final String expected_title = "Crosswalk Sample Application";

        loadAssetFile(name);
        assertEquals(expected_title, getTitleOnUiThread());
    }
}
