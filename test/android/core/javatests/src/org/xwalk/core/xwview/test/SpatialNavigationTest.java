// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;
import org.xwalk.core.XWalkSettings;

/**
 * Test suite for spatial navigation.
 */
public class SpatialNavigationTest extends XWalkViewTestBase {

    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @SmallTest
    @Feature({"Spatial navigation"})
    public void testSpatialNavigation() throws Throwable {
        XWalkSettings settings = getXWalkSettingsOnUiThreadByXWalkView(getXWalkView());

        assertTrue(settings.getSupportSpatialNavigation());
        settings.setSupportSpatialNavigation(false);
        assertFalse(settings.getBuiltInZoomControls());
        settings.setSupportSpatialNavigation(true);
        assertTrue(settings.getSupportSpatialNavigation());
    }
}
