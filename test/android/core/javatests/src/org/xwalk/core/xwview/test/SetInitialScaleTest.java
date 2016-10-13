// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.content.Context;
import android.graphics.Point;
import android.test.suitebuilder.annotation.SmallTest;
import android.view.WindowManager;

import org.chromium.base.test.util.Feature;
import org.chromium.content.browser.test.util.CallbackHelper;

import java.util.Locale;

/**
 * Test suite for setInitialScale().
 */
public class SetInitialScaleTest extends XWalkViewTestBase {
    private TestHelperBridge.OnScaleChangedHelper mOnScaleChangedHelper;

    @Override
    public void setUp() throws Exception {
        super.setUp();
        mOnScaleChangedHelper = mTestHelperBridge.getOnScaleChangedHelper();
    }

    @SmallTest
    @Feature({"setInitialScale"})
    public void testSetInitialScale1() throws Throwable {
        setQuirksMode(true);
        final String pageTemplate = "<html><head>"
                + "<meta name='viewport' content='initial-scale=%d' />"
                + "</head><body>"
                + "<div style='width:10000px;height:200px'>A big div</div>"
                + "</body></html>";
        final int initialScale4 = 4;
        final int initialScale1 = 1;
        final String pageScale4 = String.format((Locale) null, pageTemplate, initialScale4);
        final String page = String.format((Locale) null, pageTemplate, initialScale1);
        final double dipScale = getDipScale();

        // Page scale updates are asynchronous. There is an issue that we can't
        // reliably check, whether the scale as NOT changed (i.e. remains to be 1.0).
        // So we first change the scale to some non-default value, and then wait
        // until it gets back to 1.0.
        int onScaleChangedCallCount = mOnScaleChangedHelper.getCallCount();
        loadDataSync(pageScale4, "text/html", false);
        mOnScaleChangedHelper.waitForCallback(onScaleChangedCallCount);
        assertEquals(4.0f, getScaleFactor());

        // The following call to set initial scale will be ignored. However, a temporary
        // page scale change may occur, and this makes the usual onScaleChanged-based workflow
        // flaky. So instead, we are just polling the scale until it becomes 1.0.
        setInitialScale(50);
        loadDataSync(page, "text/html", false);
        ensureScaleBecomes(1.0f);
    }

    @SmallTest
    @Feature({"setInitialScale"})
    public void testSetInitialScale2() throws Throwable {
        setQuirksMode(false);
        CallbackHelper onPageFinishedHelper = mTestHelperBridge.getOnPageFinishedHelper();

        WindowManager wm = (WindowManager) getInstrumentation().getTargetContext()
                .getSystemService(Context.WINDOW_SERVICE);
        Point screenSize = new Point();
        wm.getDefaultDisplay().getSize(screenSize);
        // Make sure after 50% scale, page width still larger than screen.
        int height = screenSize.y * 2 + 1;
        int width = screenSize.x * 2 + 1;
        final String page = "<html><body>"
                + "<p style='height:" + height + "px;width:" + width + "px'>"
                + "testSetInitialScale</p></body></html>";
        final float defaultScaleFactor = 0;
        final float defaultScale = getInstrumentation().getTargetContext(
                ).getResources().getDisplayMetrics().density;

        assertEquals(defaultScaleFactor, getScaleFactor(), .01f);
        loadDataSync(page, "text/html", false);
        assertEquals(defaultScale, getPixelScale(), .01f);

        int onScaleChangedCallCount = mOnScaleChangedHelper.getCallCount();
        setInitialScale(60);
        loadDataSync(page, "text/html", false);
        mOnScaleChangedHelper.waitForCallback(onScaleChangedCallCount);
        assertEquals(0.6f, getPixelScale(), .01f);

        onScaleChangedCallCount = mOnScaleChangedHelper.getCallCount();
        setInitialScale(500);
        loadDataSync(page, "text/html", false);
        mOnScaleChangedHelper.waitForCallback(onScaleChangedCallCount);
        assertEquals(5.0f, getPixelScale(), .01f);

        onScaleChangedCallCount = mOnScaleChangedHelper.getCallCount();
        // default min-scale will be used.
        setInitialScale(0);
        loadDataSync(page, "text/html", false);
        mOnScaleChangedHelper.waitForCallback(onScaleChangedCallCount);
        assertEquals(defaultScale, getPixelScale(), .01f);
    }
}
