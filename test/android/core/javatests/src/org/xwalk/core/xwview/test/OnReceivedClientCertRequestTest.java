// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import org.xwalk.core.ClientCertRequestHandler;

import android.test.suitebuilder.annotation.MediumTest;

import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.Feature;

/**
 * Tests for the XWalkResourceClient.onReceivedClientCertRequest() method.
 */
public class OnReceivedClientCertRequestTest extends XWalkViewTestBase {
    private TestHelperBridge.OnReceivedClientCertRequestHelper mOnReceivedClientCertRequestHelper;

    @Override
    public void setUp() throws Exception {
        super.setUp();

        mOnReceivedClientCertRequestHelper = mTestHelperBridge
                .getOnReceivedClientCertRequestHelper();
    }

    // TODO(hengzhi): Since the device issue, it can not access the network,
    // so disabled this test temporarily. It will be enabled later.
    // @MediumTest
    // @Feature({ "onReceivedClientCertRequest" })
    @DisabledTest
    public void testClientCertRequest() throws Throwable {
        final String url = "https://egov.privasphere.com/";
        int onReceivedClientCertRequestCallCount = mOnReceivedClientCertRequestHelper
                .getCallCount();
        loadUrlAsync(url);

        mOnReceivedClientCertRequestHelper.waitForCallback(onReceivedClientCertRequestCallCount);
        assertEquals(ClientCertRequestHandler.class.getName(), mOnReceivedClientCertRequestHelper
                .getHandler().getClass().getName());
    }
}
