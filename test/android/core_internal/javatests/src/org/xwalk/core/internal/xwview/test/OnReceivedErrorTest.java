// Copyright 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.xwview.test;

import org.xwalk.core.internal.XWalkResourceClientInternal;

import android.test.suitebuilder.annotation.MediumTest;
import android.webkit.WebSettings;

import org.chromium.base.test.util.Feature;
import org.chromium.content.browser.test.util.TestCallbackHelperContainer;

/**
 * Tests for the XWalkResourceClientInternal.onReceivedError() method.
 */
public class OnReceivedErrorTest extends XWalkViewInternalTestBase {
    private TestCallbackHelperContainer.OnReceivedErrorHelper mOnReceivedErrorHelper;

    @Override
    public void setUp() throws Exception {
        super.setUp();

        mOnReceivedErrorHelper = mTestHelperBridge.getOnReceivedErrorHelper();
    }

    @MediumTest
    @Feature({"onReceivedError"})
    public void testCacheMiss() throws Throwable {
        final String url = "http://example.com/index.html";
        int onReceivedErrorCallCount = mOnReceivedErrorHelper.getCallCount();
        getXWalkSettingsOnUiThreadByContent(getXWalkView()).setCacheMode(WebSettings.LOAD_CACHE_ONLY);
        loadUrlAsync(url);

        mOnReceivedErrorHelper.waitForCallback(onReceivedErrorCallCount);
        assertEquals(XWalkResourceClientInternal.ERROR_UNKNOWN,
                     mOnReceivedErrorHelper.getErrorCode());
        assertEquals(url, mOnReceivedErrorHelper.getFailingUrl());
        assertFalse(mOnReceivedErrorHelper.getDescription().isEmpty());
    }
}
