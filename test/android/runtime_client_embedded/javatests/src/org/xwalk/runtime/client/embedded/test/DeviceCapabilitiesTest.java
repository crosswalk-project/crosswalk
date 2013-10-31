// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.client.embedded.test;

import android.test.suitebuilder.annotation.SmallTest;

import java.util.concurrent.TimeUnit;

import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.Feature;
import org.xwalk.test.util.OnTitleUpdatedHelper;
import org.xwalk.test.util.XWalkRuntimeClientUtilInterface;

/**
 * Test suite for W3C SysApps DeviceCapabilities API.
 */
public class DeviceCapabilitiesTest extends XWalkRuntimeClientTestBase {

    // @SmallTest
    // @Feature({"DeviceCapabilities"})
    @DisabledTest
    public void testDeviceCapabilities() throws Throwable {
        OnTitleUpdatedHelper helper = getUtilInterface().getContentsClient().getOnTitleUpdatedHelper();
        int currentCallCount = helper.getCallCount();

        getUtilInterface().loadAssetFile("device_capabilities.html");

        helper.waitForCallback(currentCallCount, 1, XWalkRuntimeClientUtilInterface.WAIT_TIMEOUT_SECONDS,
                TimeUnit.SECONDS);
        String title = helper.getTitle();
        assertEquals("Pass", title);
    }
}
