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

    // This class provides helper methods for testing of settings related to
    // the text autosizing feature.
    abstract class XWalkSettingsTextAutosizingTestHelper<T> extends XWalkSettingsTestHelper<T> {
        protected static final float PARAGRAPH_FONT_SIZE = 14.0f;
        private boolean mNeedToWaitForFontSizeChange;
        private float mOldFontSize;
        XWalkView mView;
        TestHelperBridge mBridge;

        XWalkSettingsTextAutosizingTestHelper(XWalkView view,
                TestHelperBridge bridge) throws Throwable {
            super(view);
            mNeedToWaitForFontSizeChange = false;
            mView = view;
            mBridge = bridge;
            loadDataSyncWithXWalkView(getData(), view, bridge);
        }

        @Override
        protected void setCurrentValue(T value) throws Throwable {
            mNeedToWaitForFontSizeChange = false;
            if (value != getCurrentValue()) {
                mOldFontSize = getActualFontSize();
                mNeedToWaitForFontSizeChange = true;
            }
        }

        protected float getActualFontSize() throws Throwable {
            if (!mNeedToWaitForFontSizeChange) {
                executeJavaScriptAndWaitForResultByXWalkView(
                        "setTitleToActualFontSize()", mView, mBridge);
            } else {
                final float oldFontSize = mOldFontSize;
                poll(new Callable<Boolean>() {
                    @Override
                    public Boolean call() throws Exception {
                        executeJavaScriptAndWaitForResultByXWalkView(
                                "setTitleToActualFontSize()", mView, mBridge);
                        float newFontSize = Float.parseFloat(getTitleOnUiThreadByContent(mView));
                        return newFontSize != oldFontSize;
                    }
                });
                mNeedToWaitForFontSizeChange = false;
            }
            return Float.parseFloat(getTitleOnUiThreadByContent(mView));
        }

        protected String getData() {
            DeviceDisplayInfo deviceInfo = DeviceDisplayInfo.create(mView.getContext());
            int displayWidth = (int) (deviceInfo.getDisplayWidth() / deviceInfo.getDIPScale());
            int layoutWidth = (int) (displayWidth * 2.5f); // Use 2.5 as autosizing layout tests do.
            StringBuilder sb = new StringBuilder();
            sb.append("<html>"
                    + "<head>"
                    + "<meta name=\"viewport\" content=\"width=" + layoutWidth + "\">"
                    + "<style>"
                    + "body { width: " + layoutWidth + "px; margin: 0; overflow-y: hidden; }"
                    + "</style>"
                    + "<script>"
                    + "function setTitleToActualFontSize() {"
                    // parseFloat is used to trim out the "px" suffix.
                    + "  document.title = parseFloat(getComputedStyle("
                    + "    document.getElementById('par')).getPropertyValue('font-size'));"
                    + "}</script></head>"
                    + "<body>"
                    + "<p id=\"par\" style=\"font-size:");
            sb.append(PARAGRAPH_FONT_SIZE);
            sb.append("px;\">");
            // Make the paragraph wide enough for being processed by the font autosizer.
            for (int i = 0; i < 500; i++) {
                sb.append("Hello, World! ");
            }
            sb.append("</p></body></html>");
            return sb.toString();
        }
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
