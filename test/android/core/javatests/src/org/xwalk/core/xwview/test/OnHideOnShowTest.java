// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

/**
 * Test suite for onHide(), onShow().
 */
public class OnHideOnShowTest extends XWalkViewTestBase {

    @Override
    public void setUp() throws Exception {
        super.setUp();

        setXWalkClient(new XWalkViewTestBase.TestXWalkClient());
        setXWalkWebChromeClient(new XWalkViewTestBase.TestXWalkWebChromeClient());
    }

    @SmallTest
    @Feature({"OnHideOnShow"})
    public void testOnHideOnShow() throws Throwable {
        String title = "";
        String expectedTrue = "true";
        String expectedFalse = "false";
        String url = "file:///android_asset/www/play_video.html";
        String setTitleByVideoStatus =
            "(function() {" +
            "  var video = document.getElementById('myvideo');" +
            "  if (video.paused) {" +
            "    document.title=\"false\";" +
            "  } else {" +
            "    document.title=\"true\";" +
            "  }" +
            "})();";
        String playVideo =
            "(function() {" +
            "  var video = document.getElementById('myvideo');" +
            "  if (video.paused) {" +
            "    video.play();" +
            "    document.title=\"true\";" +
            "  }" +
            "})();";

        loadUrlSync(url);

        //waitForTimerFinish(1000);
        Thread.sleep(1000);
        executeJavaScriptAndWaitForResult(setTitleByVideoStatus);
        title = getTitleOnUiThread();
        assertEquals(expectedTrue, title);

        onHideOnUiThread();
        // wait for the hide is finished.
        //waitForTimerFinish(3000);
        Thread.sleep(3000);
        executeJavaScriptAndWaitForResult(setTitleByVideoStatus);
        title = getTitleOnUiThread();
        assertEquals(expectedFalse, title);

        onShowOnUiThread();
        // wait for the show is finished.
        //waitForTimerFinish(3000);
        Thread.sleep(3000);
        executeJavaScriptAndWaitForResult(setTitleByVideoStatus);
        title = getTitleOnUiThread();
        assertEquals(expectedFalse, title);

        executeJavaScriptAndWaitForResult(playVideo);
        title = getTitleOnUiThread();
        assertEquals(expectedTrue, title);
    }
}
