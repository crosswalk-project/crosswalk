// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;

import org.chromium.base.test.util.Feature;
import org.chromium.ui.gfx.DeviceDisplayInfo;
import org.xwalk.core.XWalkView;

import java.util.concurrent.Callable;

/**
 * Test suite for setTextZoom(), getTextZoom().
 */
public class TextZoomTest extends XWalkViewTestBase {
    private final String TAG = "TextZoomTest";
    private static final float MAXIMUM_SCALE = 2.0f;
    private final float mPageMinimumScale = 0.5f;

    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    class XWalkSettingsTextZoomAutosizingTestHelper
            extends XWalkSettingsTextAutosizingTestHelper<Integer> {
        private static final int INITIAL_TEXT_ZOOM = 100;
        private final float mInitialActualFontSize;
        private final XWalkView mView;

        XWalkSettingsTextZoomAutosizingTestHelper(XWalkView view,
                TestHelperBridge bridge) throws Throwable {
            super(view, bridge);
            mView = view;
            mInitialActualFontSize = getActualFontSize();
        }

        @Override
        protected Integer getAlteredValue() {
            return INITIAL_TEXT_ZOOM * 2;
        }

        @Override
        protected Integer getInitialValue() {
            return INITIAL_TEXT_ZOOM;
        }

        @Override
        protected Integer getCurrentValue() {
            try {
                return getTextZoomOnUiThreadByXWalkView(mView);
            } catch (Exception e) {
                Log.e(TAG, "getCurrentValue e=" + e);
                return -1;
            }
        }

        @Override
        protected void setCurrentValue(Integer value) throws Throwable {
            super.setCurrentValue(value);
            setTextZoomOnUiThreadByXWalkView(value, mView);
        }

        @Override
        protected void doEnsureSettingHasValue(Integer value) throws Throwable {
            final float actualFontSize = getActualFontSize();
            // Ensure that actual vs. initial font size ratio is similar to actual vs. initial
            // text zoom values ratio.
            final float ratiosDelta = Math.abs(
                    (actualFontSize / mInitialActualFontSize)
                    - (value / (float) INITIAL_TEXT_ZOOM));
            assertTrue(
                    "|(" + actualFontSize + " / " + mInitialActualFontSize + ") - ("
                    + value + " / " + INITIAL_TEXT_ZOOM + ")| = " + ratiosDelta,
                    ratiosDelta <= 0.2f);
        }
    }

    @SmallTest
    @Feature({"setTextZoom(), getTextZoom()"})
    public void testTextZoom() throws Throwable {
        ViewPair views = createViews();
        runPerViewSettingsTest(
                new XWalkSettingsTextZoomAutosizingTestHelper(views.getView0(), 
                        views.getBridge0()),
                new XWalkSettingsTextZoomAutosizingTestHelper(views.getView1(),
                        views.getBridge1()));
    }
}
