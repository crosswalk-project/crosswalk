// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.client.embedded.test;

import android.test.suitebuilder.annotation.SmallTest;
import org.chromium.base.test.util.Feature;
import org.xwalk.runtime.client.embedded.shell.XWalkRuntimeClientEmbeddedShellActivity;
import org.xwalk.test.util.RuntimeClientApiTestBase;

/**
 * Test suite for testing native external extensions.
 */
public class NativeExternalExtensionTest extends XWalkRuntimeClientTestBase {

    @SmallTest
    @Feature({"NativeExternalExtension"})
    public void testNativeExternalExtension() throws Throwable {
        RuntimeClientApiTestBase<XWalkRuntimeClientEmbeddedShellActivity> helper =
                new RuntimeClientApiTestBase<XWalkRuntimeClientEmbeddedShellActivity>(
                        getTestUtil(), this);
        helper.testNativeExternalExtension();
    }
}
