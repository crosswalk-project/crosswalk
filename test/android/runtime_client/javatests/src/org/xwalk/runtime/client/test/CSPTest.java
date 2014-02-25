// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.client.test;

import android.test.suitebuilder.annotation.SmallTest;
import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.Feature;
import org.xwalk.runtime.client.shell.XWalkRuntimeClientShellActivity;
import org.xwalk.test.util.RuntimeClientApiTestBase;

/**
 * Test suite for CSP.
 */
public class CSPTest extends XWalkRuntimeClientTestBase {

    // TODO(gaochun): Enable the test when the issue was fixed:
    // Bug: https://crosswalk-project.org/jira/browse/XWALK-1022
    // @SmallTest
    // @Feature({"CSP"})
    @DisabledTest
    public void testCSP() throws Throwable {
        RuntimeClientApiTestBase<XWalkRuntimeClientShellActivity> helper =
                new RuntimeClientApiTestBase<XWalkRuntimeClientShellActivity>(
                        getTestUtil(), this);
        helper.testCSP();
    }
}
