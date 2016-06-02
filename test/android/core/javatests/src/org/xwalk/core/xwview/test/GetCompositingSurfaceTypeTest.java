// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;
import org.xwalk.core.XWalkPreferences;


/**
 * Test suite for getCompositingSurfaceType().
 */
public class GetCompositingSurfaceTypeTest extends XWalkViewTestBase {

    @Override
    public void setUp() throws Exception {
        XWalkPreferences.setValue(XWalkPreferences.ANIMATABLE_XWALK_VIEW, true);
        super.setUp();
    }

    @SmallTest
    @Feature({"GetCompositingSurfaceType"})
    public void testGetCompositingSurfaceType() throws Throwable {
        String surfaceType = getCompositingSurfaceTypeOnUiThread();
        assertEquals("TextureView", surfaceType);
    }
}
