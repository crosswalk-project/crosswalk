// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

/**
 * Test suite for pauseTimers(), resumeTimers().
 */
public class PauseResumeTimersTest extends XWalkViewTestBase {

    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @SmallTest
    @Feature({"PauseAndResumeTimers"})
    public void testPauseAndResumeTimers() throws Throwable {
        String title = "";
        String prevTitle = "";
        String nextTitle = "";
        String msg = "";

        title = loadAssetFileAndWaitForTitle("timer.html");
        assertNotNull(title);

        pauseTimersOnUiThread();
        msg = "The second title should be equal to the first title.";
        // wait for the pause is finished.
        waitForTimerFinish(5000);
        prevTitle = getTitleOnUiThread();
        waitForTimerFinish(200);
        nextTitle = getTitleOnUiThread();
        compareTitle(prevTitle, nextTitle, msg, Relation.EQUAL);

        resumeTimersOnUiThread();
        msg = "The second title should be greater than the first title.";
        // wait for the resume is finished.
        waitForTimerFinish(5000);
        prevTitle = getTitleOnUiThread();
        waitForTimerFinish(200);
        nextTitle = getTitleOnUiThread();
        compareTitle(prevTitle, nextTitle, msg, Relation.GREATERTHAN);
    }
}
