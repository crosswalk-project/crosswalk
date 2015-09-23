// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.Feature;

/**
 * Tests for the OnReceivedHttpAuthRequest.
 */
public class OnReceivedHttpAuthRequestTest extends XWalkViewTestBase {
    TestHelperBridge.OnReceivedHttpAuthRequestHelper mOnReceivedHttpAuthRequestHelper;

    @Override
    public void setUp() throws Exception {
        super.setUp();
        mOnReceivedHttpAuthRequestHelper = mTestHelperBridge.getOnReceivedHttpAuthRequestHelper();
    }

    // TODO(hengzhi): Since the device issue, it can not access the network,
    // so disabled this test temporarily. It will be enabled later.
    // @SmallTest
    // @Feature({"OnReceivedHttpAuthRequest"})
    @DisabledTest
    public void testOnReceivedHttpAuthRequest() throws Throwable {
        String url = "http://httpbin.org/basic-auth/user/passwd";
        String host = "httpbin.org";
        int count = mOnReceivedHttpAuthRequestHelper.getCallCount();
        loadUrlAsync(url);
        mOnReceivedHttpAuthRequestHelper.waitForCallback(count);
        assertEquals(host, mOnReceivedHttpAuthRequestHelper.getHost());
    }
}
