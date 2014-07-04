// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

/**
 * Test suite for getTitle().
 */
public class GetTitleTest extends XWalkViewTestBase {
    final String mTitle = "Crosswalk Sample Application";

    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @SmallTest
    @Feature({"GetTitle"})
    public void testGetTitle() throws Throwable {
        final String url = "file:///android_asset/www/index.html";

        loadUrlSync(url);
        assertEquals(mTitle, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"GetTitle"})
    public void testGetTitleWithData() throws Throwable {
        final String name = "index.html";
        final String fileContent = getFileContent(name);

        loadDataSync(name, fileContent, "text/html", false);
        assertEquals(mTitle, getTitleOnUiThread());
    }
}
