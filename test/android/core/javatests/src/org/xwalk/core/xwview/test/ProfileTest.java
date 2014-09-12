// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import java.io.File;
import java.io.FilenameFilter;

import android.app.Activity;
import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;
import org.xwalk.core.XWalkPreferences;

/**
 * Test suite for XWalkPreferences.PROFILE_NAME.
 */
public class ProfileTest extends XWalkViewTestBase {
    private final static String TEST_PROFILE_NAME = "test-profile";

    private boolean deleteDir(File f) {
        if (f.isDirectory()) {
            File[] files = f.listFiles();
            if (files == null) return true;
            for (File subFile : files) {
                deleteDir(subFile);
            }
        }
        return f.delete();
    }

    @Override
    public void setUp() throws Exception {
        final Activity activity = getActivity();
        String appDataDir = activity.getFilesDir().getParent();
        File userDataDir = new File(appDataDir + File.separator + "app_xwalkcore" +
                File.separator + TEST_PROFILE_NAME);
        File defaultUserDataDir = new File(appDataDir + File.separator + "app_xwalkcore" +
                File.separator + "Default");
        deleteDir(userDataDir);
        deleteDir(defaultUserDataDir);
        XWalkPreferences.setValue(XWalkPreferences.PROFILE_NAME, TEST_PROFILE_NAME);
        super.setUp();
    }

    @SmallTest
    @Feature({"Profile"})
    public void testCustomizeProfile() throws Throwable {
        final String url = "file:///android_asset/www/index.html";
        loadUrlSync(url);
        final Activity activity = getActivity();
        String appDataDir = activity.getFilesDir().getParent();
        File userDataDir = new File(appDataDir + File.separator + "app_xwalkcore" +
                File.separator + TEST_PROFILE_NAME);
        File defaultUserDataDir = new File(appDataDir + File.separator + "app_xwalkcore" +
                File.separator + "Default");
        assertTrue(userDataDir.isDirectory());
        assertFalse(defaultUserDataDir.exists());
    }
}
