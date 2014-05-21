// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

/**
 * Test suite for onLoadFinished().
 */
public class OnLoadFinishedTest extends XWalkViewTestBase {
    private TestHelperBridge.OnLoadFinishedHelper mOnLoadFinishedHelper;

    @Override
    public void setUp() throws Exception {
        super.setUp();

        setXWalkClient(new XWalkViewTestBase.TestXWalkClient());
        setResourceClient(new XWalkViewTestBase.TestXWalkResourceClient());
        mOnLoadFinishedHelper = mTestHelperBridge.getOnLoadFinishedHelper();
    }

    @SmallTest
    @Feature({"OnLoadFinished"})
    public void testOnLoadFinished() throws Throwable {
        final int callCount = mOnLoadFinishedHelper.getCallCount();
        final String url = "file:///android_asset/www/frame.html";

        loadUrlAsync(url);
        mOnLoadFinishedHelper.waitForCallback(callCount);
        assertEquals(url, mOnLoadFinishedHelper.getUrl());
    }
}
