// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.test.util;

import android.app.Activity;
import android.test.ActivityInstrumentationTestCase2;

import java.util.Timer;
import java.util.TimerTask;

/**
 * Test helper for Runtime client APIs.
 */
public class RuntimeClientApiTestBase<T extends Activity> {
    private XWalkRuntimeClientTestUtilBase mTestUtil;
    private ActivityInstrumentationTestCase2<T> mTestCase;
    private Timer mTimer = new Timer();
    enum Relation {
        EQUAL,
        GREATERTHAN,
        LESSSTHAN,
        NONE
    }

    public RuntimeClientApiTestBase(XWalkRuntimeClientTestUtilBase testUtil,
            ActivityInstrumentationTestCase2<T> testCase) {
        mTestUtil = testUtil;
        mTestCase = testCase;
    }

    public void compareTitle(String prevTitle, String title, String msg, Relation relation) {
        switch (relation) {
            case EQUAL:
                mTestCase.assertTrue(msg, title.equals(prevTitle));
                break;
            case GREATERTHAN:
                mTestCase.assertTrue(msg, title.compareTo(title) > 0);
                break;
            default:
                break;
        }
    }

    public void compareTitleAfterTimer(final String prevTitle, int milliSeconds,
            final String msg, final Relation relation) {
        mTimer.schedule(new TimerTask() {
            String title = "";
            @Override
            public void run() {
                title = mTestUtil.getTestedView().getTitleForTest();
                compareTitle(prevTitle, title, msg, relation);
            }
        }, milliSeconds);
    }

    // For loadAppFromUrl.
    public void testLoadAppFromUrl() throws Throwable {
        final String expectedTitle = "Crosswalk Sample Application";

        mTestUtil.loadUrlSync("file:///android_asset/index.html");
        mTestCase.getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                String title = mTestUtil.getTestedView().getTitleForTest();
                mTestCase.assertEquals(expectedTitle, title);
            }
        });
    }

    // For loadAppFromManifest.
    public void testLoadAppFromManifest() throws Throwable {
        final String expectedTitle = "Crosswalk Sample Application";

        mTestUtil.loadManifestSync("file:///android_asset/manifest.json");

        mTestCase.getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                String title = mTestUtil.getTestedView().getTitleForTest();
                mTestCase.assertEquals(expectedTitle, title);
            }
        });
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
        mTestCase.getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                String title = mTestUtil.getTestedView().getTitleForTest();
                mTestCase.assertEquals("Pass", title);
            }
        });
    }

    // For onPause, onResume.
    public void testPauseAndResume() throws Throwable {
        String title = "";
        String msg = "";

        title = mTestUtil.loadAssetFileAndWaitForTitle("timer.html");
        mTestCase.assertNotNull(title);

        mTestUtil.onPause();
        mTestUtil.waitForTitleUpdated(mTestUtil.WAIT_TIMEOUT_SECONDS);
        title = mTestUtil.getTestedView().getTitleForTest();
        msg = "The second title should be equal to the first title.";
        compareTitleAfterTimer(title, 200, msg, Relation.EQUAL);

        mTestUtil.onResume();
        mTestUtil.waitForTitleUpdated(mTestUtil.WAIT_TIMEOUT_SECONDS);
        title = mTestUtil.getTestedView().getTitleForTest();
        msg = "The second title should be greater than the first title.";
        compareTitleAfterTimer(title, 200, msg, Relation.GREATERTHAN);
    }
}
