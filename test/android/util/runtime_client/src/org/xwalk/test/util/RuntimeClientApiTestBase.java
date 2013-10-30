// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.test.util;

import android.app.Activity;
import android.test.ActivityInstrumentationTestCase2;

/**
 * Test helper for Runtime client APIs.
 */
public class RuntimeClientApiTestBase<T extends Activity> {
    private XWalkRuntimeClientTestUtilBase mTestUtil;
    private ActivityInstrumentationTestCase2<T> mTestCase;

    public RuntimeClientApiTestBase(XWalkRuntimeClientTestUtilBase testUtil,
            ActivityInstrumentationTestCase2<T> testCase) {
        mTestUtil = testUtil;
        mTestCase = testCase;
    }

    // For loadAppFromUrl.
    public void testLoadAppFromUrl() throws Throwable {
        String expectedTitle = "Crosswalk Sample Application";

        mTestUtil.loadUrlSync("file:///android_asset/index.html");

        String title = mTestUtil.getTestedView().getTitleForTest();
        mTestCase.assertEquals(expectedTitle, title);
    }

    // For loadAppFromManifest.
    public void testLoadAppFromManifest() throws Throwable {
        String expectedTitle = "Crosswalk Sample Application";

        mTestUtil.loadManifestSync("file:///android_asset/manifest.json");

        String title = mTestUtil.getTestedView().getTitleForTest();
        mTestCase.assertEquals(expectedTitle, title);
    }

    // For internal extension implemention of DeviceCapabilities.
    public void testDeviceCapabilities() throws Throwable {
        String title = mTestUtil.loadAssetFileAndWaitForTitle("device_capabilities.html");
        mTestCase.assertEquals("Pass", title);
    }

    // For external extension mechanism: async mode.
    public void testExternalExtensionAsync() throws Throwable {
        String title = mTestUtil.loadAssetFileAndWaitForTitle("echo.html");
        mTestCase.assertEquals("Pass", title);
    }

    // For external extension mechanism: sync mode.
    public void testExternalExtensionSync() throws Throwable {
        mTestUtil.loadAssetFile("echoSync.html");
        String title = mTestUtil.getTestedView().getTitleForTest();
        mTestCase.assertEquals("Pass", title);
    }
}
