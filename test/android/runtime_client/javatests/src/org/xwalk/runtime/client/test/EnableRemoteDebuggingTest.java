// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.client.test;

import android.content.Context;
import android.test.suitebuilder.annotation.SmallTest;
import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.Feature;
import org.xwalk.test.util.RuntimeClientApiTestBase;

/**
 * Test suite for enableRemoteDebugging().
 */
public class EnableRemoteDebuggingTest extends XWalkRuntimeClientTestBase {
    @SmallTest
    @Feature({"EnableRemoteDebugging"})
    public void testEnableRemoteDebugging() throws Throwable {
        Context context = getActivity();
        RuntimeClientApiTestBase<XWalkRuntimeClientTestRunnerActivity> helper =
                new RuntimeClientApiTestBase<XWalkRuntimeClientTestRunnerActivity>(
                        getTestUtil(), this);
        helper.testEnableRemoteDebugging(getActivity(), context);
    }

    // This test case failed on trybot, but passed on buildbot and local machine.
    // Disabled it first, It will be enabled later.
    // @SmallTest
    // @Feature({"DisableRemoteDebugging"})
    @DisabledTest
    public void testDisableRemoteDebugging() throws Throwable {
        Context context = getActivity();
        RuntimeClientApiTestBase<XWalkRuntimeClientTestRunnerActivity> helper =
                new RuntimeClientApiTestBase<XWalkRuntimeClientTestRunnerActivity>(
                        getTestUtil(), this);
        helper.testDisableRemoteDebugging(getActivity(), context);
    }
}
