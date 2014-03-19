// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.graphics.Bitmap;
import android.test.suitebuilder.annotation.SmallTest;
import android.webkit.WebResourceResponse;

import java.util.concurrent.atomic.AtomicReference;

import org.chromium.base.test.util.Feature;
import org.chromium.base.test.util.UrlUtils;
import org.xwalk.core.XWalkClient;
import org.xwalk.core.XWalkContent;
import org.xwalk.core.XWalkSettings;
import org.xwalk.core.XWalkView;
import org.xwalk.core.XWalkWebChromeClient;

/**
 * Test suite for setDomStorageEnabled().
 */
public class SetDomStorageEnabledTest extends XWalkViewTestBase {
    private static final boolean ENABLED = true;
    private static final boolean DISABLED = false;

    abstract class XWalkViewSettingsTestHelper<T> {
        protected final XWalkContent mXWalkContent;
        protected final XWalkSettings mXWalkSettings;

        XWalkViewSettingsTestHelper(XWalkContent xWalkContent,
                boolean requiresJsEnabled) throws Throwable {
            mXWalkContent = xWalkContent;
            mXWalkSettings = getXWalkSettingsOnUiThreadByContent(xWalkContent);
            mXWalkSettings.setDomStorageEnabled(false);
            if (requiresJsEnabled) {
                mXWalkSettings.setJavaScriptEnabled(true);
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

    class XWalkViewSettingsDomStorageEnabledTestHelper extends XWalkViewSettingsTestHelper<Boolean> {
        private static final String NO_LOCAL_STORAGE = "No localStorage";
        private static final String HAS_LOCAL_STORAGE = "Has localStorage";
        TestHelperBridge mHelperBridge;

        XWalkViewSettingsDomStorageEnabledTestHelper(
                XWalkContent xWalkContent,
                final TestHelperBridge helperBridge) throws Throwable {
            super(xWalkContent, true);
            mHelperBridge = helperBridge;
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
            return mXWalkSettings.getDomStorageEnabled();
        }

        @Override
        protected void setCurrentValue(Boolean value) {
            mXWalkSettings.setDomStorageEnabled(value);
        }

        @Override
        protected void doEnsureSettingHasValue(Boolean value) throws Throwable {
            // It is not permitted to access localStorage from data URLs in WebKit,
            // that is why a standalone page must be used.
            loadUrlSyncByContent(mXWalkContent, mHelperBridge,
                    UrlUtils.getTestFileUrl("xwalkview/localStorage.html"));
            assertEquals(
                value == ENABLED ? HAS_LOCAL_STORAGE : NO_LOCAL_STORAGE,
                        mHelperBridge.getChangedTitle());
        }
    }

    /**
     * Verifies the following statements about a setting:
     *  - initially, the setting has a default value;
     *  - the setting can be switched to an alternate value and back;
     *  - switching a setting in the first XWalkView doesn't affect the setting
     *    state in the second XWalkView and vice versa.
     *
     * @param helper0 Test helper for the first ContentView
     * @param helper1 Test helper for the second ContentView
     * @throws Throwable
     */
    private void runPerViewSettingsTest(XWalkViewSettingsTestHelper helper0,
            XWalkViewSettingsTestHelper helper1) throws Throwable {
        helper0.ensureSettingHasInitialValue();
        helper1.ensureSettingHasInitialValue();

        helper1.setAlteredSettingValue();
        helper0.ensureSettingHasInitialValue();
        helper1.ensureSettingHasAlteredValue();

        helper1.setInitialSettingValue();
        helper0.ensureSettingHasInitialValue();
        helper1.ensureSettingHasInitialValue();

        helper0.setAlteredSettingValue();
        helper0.ensureSettingHasAlteredValue();
        helper1.ensureSettingHasInitialValue();

        helper0.setInitialSettingValue();
        helper0.ensureSettingHasInitialValue();
        helper1.ensureSettingHasInitialValue();

        helper0.setAlteredSettingValue();
        helper0.ensureSettingHasAlteredValue();
        helper1.ensureSettingHasInitialValue();

        helper1.setAlteredSettingValue();
        helper0.ensureSettingHasAlteredValue();
        helper1.ensureSettingHasAlteredValue();

        helper0.setInitialSettingValue();
        helper0.ensureSettingHasInitialValue();
        helper1.ensureSettingHasAlteredValue();

        helper1.setInitialSettingValue();
        helper0.ensureSettingHasInitialValue();
        helper1.ensureSettingHasInitialValue();
    }

    @SmallTest
    @Feature({"XWalkView", "Preferences"})
    public void testDomStorageEnabledWithTwoViews() throws Throwable {
        ViewPair views = createViews();
        runPerViewSettingsTest(
            new XWalkViewSettingsDomStorageEnabledTestHelper(
                    views.getContent0(), views.getClient0()),
            new XWalkViewSettingsDomStorageEnabledTestHelper(
                    views.getContent1(), views.getClient1()));
    }
}
