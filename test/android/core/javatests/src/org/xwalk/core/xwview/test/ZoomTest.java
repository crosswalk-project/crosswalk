// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;

import java.util.Locale;

import org.chromium.base.test.util.Feature;
import org.xwalk.core.XWalkSettings;

/**
 * Test suite for zoom related functions.
 */
public class ZoomTest extends XWalkViewTestBase {
    private static final float MAXIMUM_SCALE = 2.0f;

    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    private String getZoomableHtml(float scale) {
        final int divWidthPercent = (int) (100.0f / scale);
        return String.format(Locale.US, "<html><head><meta name=\"viewport\" content=\""
                + "width=device-width, minimum-scale=%f, maximum-scale=%f, initial-scale=%f"
                + "\"/></head><body style='margin:0'>"
                + "<div style='width:%d%%;height:100px;border:1px solid black'>Zoomable</div>"
                + "</body></html>",
                scale, MAXIMUM_SCALE, scale, divWidthPercent);
    }

    @SmallTest
    @Feature({"Zoom test"})
    public void testZoomUsingMultiTouch() throws Throwable {
        XWalkSettings settings = getXWalkSettingsOnUiThreadByXWalkView(getXWalkView());
        loadDataSync(getZoomableHtml(0.5f), "text/html", false);

        assertTrue(settings.supportZoom());
        assertFalse(settings.getBuiltInZoomControls());
        assertFalse(settings.supportsMultiTouchZoomForTest());

        settings.setBuiltInZoomControls(true);
        assertTrue(settings.supportsMultiTouchZoomForTest());

        settings.setSupportZoom(false);
        assertFalse(settings.supportsMultiTouchZoomForTest());
    }
}
