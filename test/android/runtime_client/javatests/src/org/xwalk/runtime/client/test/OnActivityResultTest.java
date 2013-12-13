// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.client.test;

import android.content.Context;
import android.test.suitebuilder.annotation.SmallTest;
import org.chromium.base.test.util.Feature;
import org.xwalk.test.util.RuntimeClientApiTestBase;

/**
 * Test suite for onActivityResult().
 */
public class OnActivityResultTest extends XWalkRuntimeClientTestBase {

    @SmallTest
    @Feature({"OnActivityResult"})
    public void testOnActivityResult() throws Throwable {
        RuntimeClientApiTestBase<XWalkRuntimeClientTestRunnerActivity> helper =
                new RuntimeClientApiTestBase<XWalkRuntimeClientTestRunnerActivity>(
                        getTestUtil(), this);
        XWalkRuntimeClientTestRunnerActivity clientActivity = getActivity();
        Context context = getActivity();

        helper.testOnActivityResult(clientActivity, context);
    }
}
