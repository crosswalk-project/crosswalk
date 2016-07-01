// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.MediumTest;
import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;
import org.xwalk.core.XWalkSettings;

/**
 * Test suite for XWalkSettings
 */
public class SettingsTest extends XWalkViewTestBase {
    private static final String USER_AGENT =
            "Chrome/44.0.2403.81 Crosswalk/15.44.376.0 Mobile Safari/537.36";
    private static final String LANGUAGE = "jp";

    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @MediumTest
    @Feature({"Settings"})
    public void testUserAgentString() throws Throwable {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                XWalkSettings settings = getXWalkView().getSettings();
                String defaultUserAgentString = settings.getUserAgentString();

                // Check that an attempt to set the default UA string to null or "" has no effect.
                settings.setUserAgentString(null);
                assertEquals(defaultUserAgentString, settings.getUserAgentString());
                settings.setUserAgentString("");
                assertEquals(defaultUserAgentString, settings.getUserAgentString());

                // Set a custom UA string, verify that it can be reset back to default.
                settings.setUserAgentString(USER_AGENT);
                assertEquals(USER_AGENT, settings.getUserAgentString());
                settings.setUserAgentString(null);
                assertEquals(defaultUserAgentString, settings.getUserAgentString());
            }
        });
    }

    @MediumTest
    @Feature({"Settings"})
    public void testAcceptLanguages() throws Throwable {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                XWalkSettings settings = getXWalkView().getSettings();
                String defaultLanguages = settings.getAcceptLanguages();

                // Set a custom UA string, verify that it can be reset back to default.
                settings.setAcceptLanguages(LANGUAGE);
                assertEquals(LANGUAGE, settings.getAcceptLanguages());
                settings.setAcceptLanguages(defaultLanguages);
                assertEquals(defaultLanguages, settings.getAcceptLanguages());
            }
        });
    }

    @SmallTest
    @Feature({"Settings"})
    public void testSaveFormData() throws Throwable {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                XWalkSettings settings = getXWalkView().getSettings();
                boolean defaultvalue = settings.getSaveFormData();

                settings.setSaveFormData(false);
                assertEquals(false, settings.getSaveFormData());
                settings.setSaveFormData(defaultvalue);
                assertEquals(defaultvalue, settings.getSaveFormData());
            }
        });
    }
}
