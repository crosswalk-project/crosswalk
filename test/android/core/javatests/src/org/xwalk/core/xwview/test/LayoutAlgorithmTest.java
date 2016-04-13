// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;

import org.chromium.base.test.util.Feature;
import org.xwalk.core.XWalkSettings;
import org.xwalk.core.XWalkSettings.LayoutAlgorithm;
import org.xwalk.core.XWalkView;

/**
 * Test suite for LayoutAlgorithm related functions.
 */
public class LayoutAlgorithmTest extends XWalkViewTestBase {
    private final String TAG = "LayoutAlgorithmTest";
    private static final float MAXIMUM_SCALE = 2.0f;

    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    class XWalkSettingsLayoutAlgorithmTestHelper extends
            XWalkSettingsTextAutosizingTestHelper<LayoutAlgorithm> {
        private final XWalkView mView;

        XWalkSettingsLayoutAlgorithmTestHelper(XWalkView view,
                TestHelperBridge bridge) throws Throwable {
            super(view, bridge);
            mView = view;
            // Font autosizing doesn't step in for narrow layout widths.
            setUseWideViewPortOnUiThreadByXWalkView(true, mView);
        }

        @Override
        protected LayoutAlgorithm getAlteredValue() {
            return LayoutAlgorithm.TEXT_AUTOSIZING;
        }

        @Override
        protected LayoutAlgorithm getInitialValue() {
            return LayoutAlgorithm.NARROW_COLUMNS;
        }

        @Override
        protected LayoutAlgorithm getCurrentValue() {
            try {
                return getLayoutAlgorithmOnUiThreadByXWalkView(mView);
            } catch (Exception e) {
                Log.e(TAG, "getCurrentValue e=" + e, new Exception());
                return LayoutAlgorithm.NARROW_COLUMNS;
            }
        }

        @Override
        protected void setCurrentValue(LayoutAlgorithm value) throws Throwable {
            super.setCurrentValue(value);
            setLayoutAlgorithmOnUiThreadByXWalkView(value, mView);
        }

        @Override
        protected void doEnsureSettingHasValue(LayoutAlgorithm value) throws Throwable {
            final float actualFontSize = getActualFontSize();
            if (value == LayoutAlgorithm.TEXT_AUTOSIZING) {
                assertFalse("Actual font size: " + actualFontSize,
                        actualFontSize == PARAGRAPH_FONT_SIZE);
            } else {
                assertTrue("Actual font size: " + actualFontSize,
                        actualFontSize == PARAGRAPH_FONT_SIZE);
            }
        }
    }

    @SmallTest
    @Feature({"LayoutAlgorithm test"})
    public void testLayoutAlgorithmWithTwoViews() throws Throwable {
        ViewPair views = createViews();
        runPerViewSettingsTest(
                new XWalkSettingsLayoutAlgorithmTestHelper(views.getView0(), views.getBridge0()),
                new XWalkSettingsLayoutAlgorithmTestHelper(views.getView1(), views.getBridge1()));
    }
}
