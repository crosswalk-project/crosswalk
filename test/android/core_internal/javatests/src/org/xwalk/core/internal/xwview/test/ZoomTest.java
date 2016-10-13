// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;

import java.util.Locale;

import org.chromium.base.test.util.Feature;

/**
 * Test suite for zoomBy(), zoomIn(), zoomOut().
 */
public class ZoomTest extends XWalkViewInternalTestBase {
    private static final float MAXIMUM_SCALE = 2.0f;
    private final float mPageMinimumScale = 0.5f;

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
    @Feature({"zoomBy, zoomIn, zoomOut"})
    public void testZoom() throws Throwable {
        setUseWideViewPortOnUiThreadByXWalkView(true, getXWalkView());
        assertFalse("Should not be able to zoom in", canZoomInOnUiThread());

        loadDataSync(getZoomableHtml(mPageMinimumScale), "text/html", false);
        waitForScaleToBecome(mPageMinimumScale);
        assertTrue("Should be able to zoom in", canZoomInOnUiThread());
        assertFalse("Should not be able to zoom out", canZoomOutOnUiThread());

        while (canZoomInOnUiThread()) {
            zoomInOnUiThreadAndWait();
        }
        assertTrue("Should be able to zoom out", canZoomOutOnUiThread());

        while (canZoomOutOnUiThread()) {
            zoomOutOnUiThreadAndWait();
        }
        assertTrue("Should be able to zoom in", canZoomInOnUiThread());

        zoomByOnUiThreadAndWait(4.0f);
        waitForScaleToBecome(MAXIMUM_SCALE);

        zoomByOnUiThreadAndWait(0.5f);
        waitForScaleToBecome(MAXIMUM_SCALE * 0.5f);

        zoomByOnUiThreadAndWait(0.01f);
        waitForScaleToBecome(mPageMinimumScale);
    }
}
