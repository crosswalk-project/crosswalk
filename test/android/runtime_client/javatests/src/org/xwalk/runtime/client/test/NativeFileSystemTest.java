// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.client.test;

import android.test.suitebuilder.annotation.SmallTest;
import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.Feature;
import org.xwalk.runtime.client.shell.XWalkRuntimeClientShellActivity;
import org.xwalk.test.util.RuntimeClientApiTestBase;

/**
 * Test suite for Native File System API.
 */
public class NativeFileSystemTest extends XWalkRuntimeClientTestBase {
    // @SmallTest
    // @Feature({"NativeFileSystem"})
    @DisabledTest
    public void testNativeFileSystem() throws Throwable {
        RuntimeClientApiTestBase<XWalkRuntimeClientShellActivity> helper =
                new RuntimeClientApiTestBase<XWalkRuntimeClientShellActivity>(
                        getTestUtil(), this);
        helper.testNativeFileSystem();
    }
}
