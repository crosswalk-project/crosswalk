// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;
import org.xwalk.core.XWalkSettings;
import org.xwalk.core.XWalkView;

/**
 * Test suite for setDefaultFontSize(), getDefaultFontSize().
 */
public class DefaultFontSizeTest extends XWalkViewTestBase {

    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    class XWalkSettingsDefaultFontSizeTestHelper extends XWalkSettingsTestHelper<Integer> {
        TestHelperBridge mBridge;
        XWalkView mView;

        XWalkSettingsDefaultFontSizeTestHelper(XWalkView view,
                TestHelperBridge bridge) throws Throwable {
            super(view);
            mView = view;
            mBridge = bridge;
        }

        @Override
        protected Integer getAlteredValue() {
            return 42;
        }

        @Override
        protected Integer getInitialValue() {
            return 16;
        }

        @Override
        protected Integer getCurrentValue() {
            return mXWalkSettingsForHelper.getDefaultFontSize();
        }

        @Override
        protected void setCurrentValue(Integer value) {
            mXWalkSettingsForHelper.setDefaultFontSize(value);
        }

        @Override
        protected void doEnsureSettingHasValue(Integer value) throws Throwable {
            loadDataSyncWithXWalkView(getData(), mView, mBridge);
            assertEquals(value.toString() + "px", getTitleOnUiThreadByContent(mView));
        }

        private String getData() {
            return "<html><body onload=\"document.title = "
                    + "getComputedStyle(document.body).getPropertyValue('font-size');\">"
                    + "</body></html>";
        }
    }

    @SmallTest
    @Feature({"setDefaultFontSize(), getDefaultFontSize()"})
    public void testDefaultFontSizeWithTwoViews() throws Throwable {
        ViewPair views = createViews();
        runPerViewSettingsTest(
                new XWalkSettingsDefaultFontSizeTestHelper(views.getView0(), views.getBridge0()),
                new XWalkSettingsDefaultFontSizeTestHelper(views.getView1(), views.getBridge1()));
    }

    @SmallTest
    @Feature({"setDefaultFontSize(), getDefaultFontSize()"})
    public void testAccessDefaultFontSize() throws Throwable {
        XWalkSettings settings = getXWalkSettingsOnUiThreadByXWalkView(getXWalkView());
        int defaultSize = settings.getDefaultFontSize();
        assertTrue(defaultSize > 0);
        settings.setDefaultFontSize(1000);
        int maxSize = settings.getDefaultFontSize();
        assertTrue(maxSize > defaultSize);
        settings.setDefaultFontSize(-10);
        int minSize = settings.getDefaultFontSize();
        assertTrue(minSize > 0);
        assertTrue(minSize < maxSize);
        settings.setDefaultFontSize(10);
        assertEquals(10, settings.getDefaultFontSize());
    }
}
