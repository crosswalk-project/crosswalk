// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.content_shell_apk;

import android.content.Context;
import android.os.PowerManager;
import android.test.suitebuilder.annotation.Smoke;

import org.chromium.base.test.util.Feature;

/**
 * Test that verifies preconditions for tests to run.
 */
public class ContentShellPreconditionsTest extends ContentShellTestBase {
    @Smoke
    @Feature({"TestInfrastructure"})
    public void testScreenIsOn() throws Exception {
        PowerManager pm = (PowerManager) getInstrumentation().getContext().getSystemService(
                Context.POWER_SERVICE);

        assertTrue("Many tests will fail if the screen is not on.", pm.isScreenOn());
    }
}
