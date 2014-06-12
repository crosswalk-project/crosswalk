// Copyright 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import org.xwalk.core.XWalkResourceClient;

import android.test.suitebuilder.annotation.MediumTest;
import android.webkit.WebSettings;

import org.chromium.base.test.util.Feature;
import org.chromium.content.browser.test.util.TestCallbackHelperContainer;

import java.util.concurrent.TimeUnit;

/**
 * Tests for the XWalkResourceClient.onReceivedError() method.
 */
public class OnReceivedErrorTest extends XWalkViewTestBase {
    private TestCallbackHelperContainer.OnReceivedErrorHelper mOnReceivedErrorHelper;

    @Override
    public void setUp() throws Exception {
        super.setUp();

        mOnReceivedErrorHelper = mTestHelperBridge.getOnReceivedErrorHelper();
        setXWalkClient(new XWalkViewTestBase.TestXWalkClient());
        setResourceClient(new XWalkViewTestBase.TestXWalkResourceClient());
    }

    @MediumTest
    @Feature({"onReceivedError"})
    public void testOnReceivedErrorOnInvalidUrl() throws Throwable {
        String url = "http://man.id.be.really.surprised.if.this.address.existed.blah/";
        int onReceivedErrorCallCount = mOnReceivedErrorHelper.getCallCount();
        loadUrlAsync(url);

        mOnReceivedErrorHelper.waitForCallback(onReceivedErrorCallCount);
        assertEquals(XWalkResourceClient.ERROR_HOST_LOOKUP,
                mOnReceivedErrorHelper.getErrorCode());
        assertEquals(url, mOnReceivedErrorHelper.getFailingUrl());
        assertNotNull(mOnReceivedErrorHelper.getDescription());
    }

    @MediumTest
    @Feature({"onReceivedError"})
    public void testOnReceivedErrorOnInvalidScheme() throws Throwable {
        String url = "foo://some/resource";
        int onReceivedErrorCallCount = mOnReceivedErrorHelper.getCallCount();
        loadUrlAsync(url);

        mOnReceivedErrorHelper.waitForCallback(onReceivedErrorCallCount);
        assertEquals(XWalkResourceClient.ERROR_UNSUPPORTED_SCHEME,
                mOnReceivedErrorHelper.getErrorCode());
        assertEquals(url, mOnReceivedErrorHelper.getFailingUrl());
        assertNotNull(mOnReceivedErrorHelper.getDescription());
    }

    @MediumTest
    @Feature({"onReceivedError"})
    public void testNoErrorOnFailedSubresourceLoad() throws Throwable {
        TestCallbackHelperContainer.OnPageFinishedHelper onPageFinishedHelper =
                mTestHelperBridge.getOnPageFinishedHelper();

        int currentCallCount = onPageFinishedHelper.getCallCount();
        loadDataAsync("<html><iframe src=\"http//invalid.url.co/\" /></html>",
                      null,
                      "text/html",
                      false);

        onPageFinishedHelper.waitForCallback(currentCallCount);
        assertEquals(0, mOnReceivedErrorHelper.getCallCount());
    }

    @MediumTest
    @Feature({"onReceivedError"})
    public void testNonExistentAssetUrl() throws Throwable {
        final String url = "file:///android_asset/does_not_exist.html";
        int onReceivedErrorCallCount = mOnReceivedErrorHelper.getCallCount();
        loadUrlAsync(url);

        mOnReceivedErrorHelper.waitForCallback(onReceivedErrorCallCount);
        assertEquals(XWalkResourceClient.ERROR_UNKNOWN,
                     mOnReceivedErrorHelper.getErrorCode());
        assertEquals(url, mOnReceivedErrorHelper.getFailingUrl());
        assertNotNull(mOnReceivedErrorHelper.getDescription());
    }

    @MediumTest
    @Feature({"onReceivedError"})
    public void testNonExistentResourceUrl() throws Throwable {
        final String url = "file:///android_res/raw/does_not_exist.html";
        int onReceivedErrorCallCount = mOnReceivedErrorHelper.getCallCount();
        loadUrlAsync(url);

        mOnReceivedErrorHelper.waitForCallback(onReceivedErrorCallCount);
        assertEquals(XWalkResourceClient.ERROR_UNKNOWN,
                     mOnReceivedErrorHelper.getErrorCode());
        assertEquals(url, mOnReceivedErrorHelper.getFailingUrl());
        assertNotNull(mOnReceivedErrorHelper.getDescription());
    }

    @MediumTest
    @Feature({"onReceivedError"})
    public void testCacheMiss() throws Throwable {
        final String url = "http://example.com/index.html";
        int onReceivedErrorCallCount = mOnReceivedErrorHelper.getCallCount();
        getXWalkSettingsOnUiThreadByContent(getXWalkView()).setCacheMode(WebSettings.LOAD_CACHE_ONLY);
        loadUrlAsync(url);

        mOnReceivedErrorHelper.waitForCallback(onReceivedErrorCallCount);
        assertEquals(XWalkResourceClient.ERROR_UNKNOWN,
                     mOnReceivedErrorHelper.getErrorCode());
        assertEquals(url, mOnReceivedErrorHelper.getFailingUrl());
        assertFalse(mOnReceivedErrorHelper.getDescription().isEmpty());
    }
}
