// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.client.embedded.test;

import android.test.suitebuilder.annotation.SmallTest;
import org.chromium.base.test.util.Feature;
import org.xwalk.test.util.RuntimeClientApiTestBase;

/**
 * Test suite for onPause(), onResume().
 */
public class PauseResumeTest extends XWalkRuntimeClientTestBase {

    @SmallTest
    @Feature({"PauseResume"})
    public void testPauseAndResume() throws Throwable {
        RuntimeClientApiTestBase<XWalkRuntimeClientTestRunnerActivity> helper =
                new RuntimeClientApiTestBase<XWalkRuntimeClientTestRunnerActivity>(
                        getTestUtil(), this);
        helper.testPauseAndResume();
    }
}
