// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;
import org.chromium.base.test.util.Feature;
import org.xwalk.core.internal.xwview.test.ExtensionBroadcastInternal;

/**
 * Test suite for ExtensionBroadcastInternal().
 */
public class ExtensionBroadcastInternalTest extends XWalkViewInternalTestBase {
    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @SmallTest
    @Feature({"ExtensionBroadcastInternal"})
    public void test() throws Throwable {
        ExtensionBroadcastInternal broadcast = new ExtensionBroadcastInternal();

        loadAssetFileAndWaitForTitle("broadcast.html");
        assertEquals("Pass", getTitleOnUiThread());
    }
}
