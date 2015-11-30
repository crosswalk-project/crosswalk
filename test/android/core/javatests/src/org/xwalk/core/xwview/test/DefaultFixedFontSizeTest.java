// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;
import org.xwalk.core.XWalkSettings;

/**
 * Test suite for setDefaultFixedFontSize(), getDefaultFixedFontSize().
 */
public class DefaultFixedFontSizeTest extends XWalkViewTestBase {

    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @SmallTest
    @Feature({"setDefaultFixedFontSize(), getDefaultFixedFontSize()"})
    public void testAccessDefaultFixedFontSize() throws Throwable {
        XWalkSettings settings = getXWalkSettingsOnUiThreadByXWalkView(getXWalkView());
        int defaultSize = settings.getDefaultFixedFontSize();
        assertTrue(defaultSize > 0);
        settings.setDefaultFixedFontSize(1000);
        int maxSize = settings.getDefaultFixedFontSize();
        assertTrue(maxSize > defaultSize);
        settings.setDefaultFixedFontSize(-10);
        int minSize = settings.getDefaultFixedFontSize();
        assertTrue(minSize > 0);
        assertTrue(minSize < maxSize);
        settings.setDefaultFixedFontSize(10);
        assertEquals(10, settings.getDefaultFixedFontSize());
    }
}
