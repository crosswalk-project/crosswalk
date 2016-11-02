// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.xwview.test;

import android.test.suitebuilder.annotation.MediumTest;

import java.lang.Thread;

import junit.framework.Assert;

import org.chromium.base.test.util.Feature;
import org.chromium.content.browser.ContentViewCore;
import org.chromium.content.browser.test.util.DOMUtils;
import org.chromium.content.browser.test.util.TouchCommon;
import org.xwalk.core.internal.xwview.test.util.VideoTestWebServer;

/**
 * Tests XWalkUIClient::onShow/onHideCustomView.
 */
public class OnShowOnHideCustomViewTest extends XWalkViewInternalTestBase {
    private static final int MAX_CLICKS = 5;

    private VideoTestWebServer mWebServer;
    private TestXWalkUIClientInternal mUIClient;
    private ContentViewCore mContentViewCore;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mUIClient = new XWalkViewInternalTestBase.TestXWalkUIClientInternal();
        setUIClient(mUIClient);
        mContentViewCore = getContentViewCore();
        mWebServer = new VideoTestWebServer(getInstrumentation().getContext());
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        if (mWebServer != null) mWebServer.getTestWebServer().shutdown();
    }

    @MediumTest
    @Feature({"onShow/onHideCustomView"})
    public void testOnShowAndHideCustomViewWithJavascript() throws Throwable {
        doOnShowAndHideCustomViewTest(new Runnable() {
            @Override
            public void run() {
                DOMUtils.exitFullscreen(mContentViewCore.getWebContents());
            }
        });
    }

    @MediumTest
    @Feature({"onShow/onHideCustomView"})
    public void testOnShowCustomViewAndPlayWithHtmlControl() throws Throwable {
        doOnShowCustomViewTest();
        Assert.assertTrue(DOMUtils.isMediaPaused(mContentViewCore.getWebContents(),
                                                 VideoTestWebServer.VIDEO_ID));

        // Click the html play button that is rendered above the video right in the middle
        // of the custom view. Note that we're not able to get the precise location of the
        // control since it is a shadow element, so this test might break if the location
        // ever moves.
        for (int i = 0; i < MAX_CLICKS; ++i) {
            Thread.sleep(1000);
            TouchCommon.singleClickView(mUIClient.getCustomView());
        }

        DOMUtils.waitForMediaPlay(
                mContentViewCore.getWebContents(), VideoTestWebServer.VIDEO_ID);
    }

    private void doOnShowAndHideCustomViewTest(final Runnable existFullscreen) throws Throwable {
        doOnShowCustomViewTest();
        getInstrumentation().runOnMainSync(existFullscreen);
        mUIClient.waitForCustomViewHidden();
    }

    private void doOnShowCustomViewTest() throws Exception {
        loadTestPageAndClickFullscreen();
        mUIClient.waitForCustomViewShown();
    }

    private void loadTestPageAndClickFullscreen() throws Exception {
        loadUrlSync(mWebServer.getFullScreenVideoTestURL());

        // Click the button in full_screen_video_test.html to enter fullscreen.
        TouchCommon.singleClickView(getXWalkView());
    }
}
