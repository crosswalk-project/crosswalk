// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.xwview.test;

import android.graphics.Bitmap;
import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;
import android.webkit.WebResourceResponse;

import org.chromium.base.test.util.Feature;
import org.chromium.base.test.util.UrlUtils;
import org.xwalk.core.internal.XWalkClient;
import org.xwalk.core.internal.XWalkViewInternal;

/**
 * Test suite for setDatabaseEnabled().
 */
public class SetDatabaseEnabledTest extends XWalkViewInternalTestBase {
    private static final boolean ENABLED = true;
    private static final boolean DISABLED = false;

    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    abstract class XWalkViewSettingsTestHelper<T> {
        XWalkViewSettingsTestHelper(boolean requiresJsEnabled) throws Throwable {
            if (requiresJsEnabled) {
                getInstrumentation().runOnMainSync(new Runnable() {
                    @Override
                    public void run() {
                        getXWalkView().getSettings().setJavaScriptEnabled(true);
                    }
                });
            }
        }

        void ensureSettingHasAlteredValue() throws Throwable {
            ensureSettingHasValue(getAlteredValue());
        }

        void ensureSettingHasInitialValue() throws Throwable {
            ensureSettingHasValue(getInitialValue());
        }

        void setAlteredSettingValue() throws Throwable {
            setCurrentValue(getAlteredValue());
        }

        void setInitialSettingValue() throws Throwable {
            setCurrentValue(getInitialValue());
        }

        protected abstract T getAlteredValue();

        protected abstract T getInitialValue();

        protected abstract T getCurrentValue();

        protected abstract void setCurrentValue(T value) throws Throwable;

        protected abstract void doEnsureSettingHasValue(T value) throws Throwable;

        private void ensureSettingHasValue(T value) throws Throwable {
            assertEquals(value, getCurrentValue());
            doEnsureSettingHasValue(value);
        }
    }

    class XWalkViewSettingsDatabaseTestHelper extends XWalkViewSettingsTestHelper<Boolean> {
        private static final String NO_DATABASE = "No database";
        private static final String HAS_DATABASE = "Has database";
        private Boolean enabled;

        XWalkViewSettingsDatabaseTestHelper() throws Throwable {
            super(true);
        }

        @Override
        protected Boolean getAlteredValue() {
            return ENABLED;
        }

        @Override
        protected Boolean getInitialValue() {
            return DISABLED;
        }

        @Override
        protected Boolean getCurrentValue() {
            getInstrumentation().runOnMainSync(new Runnable() {
                @Override
                public void run() {
                    enabled = getXWalkView().getSettings().getDatabaseEnabled();
                }
            });
            return enabled;
        }

        @Override
        protected void setCurrentValue(Boolean value) {
            final Boolean enabled = value;
            getInstrumentation().runOnMainSync(new Runnable() {
                @Override
                public void run() {
                    getXWalkView().getSettings().setDatabaseEnabled(enabled);
                }
            });
        }

        @Override
        protected void doEnsureSettingHasValue(Boolean value) throws Throwable {
            // It seems accessing the database through a data scheme is not
            // supported, and fails with a DOM exception (likely a cross-domain
            // violation).
            String path = "xwalk/test/android/data/device_files/database_access.html";
            String url = UrlUtils.getIsolatedTestFileUrl(path);
            loadUrlSync(url);
            assertEquals(
                value == ENABLED ? HAS_DATABASE : NO_DATABASE,
                getTitleOnUiThread());
        }
    }

    @SmallTest
    @Feature({"XWalkViewInternal", "Preferences"})
    public void testDatabaseEnabled() throws Throwable {
        XWalkViewSettingsDatabaseTestHelper helper = new XWalkViewSettingsDatabaseTestHelper();
        helper.setAlteredSettingValue();
        helper.ensureSettingHasAlteredValue();
    }

    @SmallTest
    @Feature({"XWalkViewInternal", "Preferences"})
    public void testDatabaseDisabled() throws Throwable {
        XWalkViewSettingsDatabaseTestHelper helper = new XWalkViewSettingsDatabaseTestHelper();
        helper.setInitialSettingValue();
        helper.ensureSettingHasInitialValue();
    }
}
