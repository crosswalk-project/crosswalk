// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.graphics.Bitmap;
import android.content.Context;
import android.view.View;
import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;
import android.widget.FrameLayout;
import java.util.concurrent.Callable;
import org.chromium.base.test.util.Feature;
import org.chromium.content.browser.ContentViewCore;
import org.xwalk.core.XWalkClient;
import org.xwalk.core.XWalkContent;
import org.xwalk.core.XWalkContentsClient;
import org.xwalk.core.XWalkSettings;
import org.xwalk.core.XWalkView;
import org.xwalk.core.XWalkWebChromeClient;

/**
 * Test suite for setUserAgentString() and getUserAgentString().
 */
public class UserAgentTest extends XWalkViewTestBase {
    @Override
    public void setUp() throws Exception {
        super.setUp();

        class TestXWalkClient extends XWalkClient {
            @Override
            public void onPageStarted(XWalkView view, String url, Bitmap favicon) {
                mTestContentsClient.onPageStarted(url);
            }

            @Override
            public void onPageFinished(XWalkView view, String url) {
                mTestContentsClient.didFinishLoad(url);
            }
        }

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getXWalkView().setXWalkClient(new TestXWalkClient());
            }
        });
    }

    protected XWalkSettings getXWalkSettingsOnUiThread(
            ) throws Exception {
        return runTestOnUiThreadAndGetResult(new Callable<XWalkSettings>() {
            @Override
            public XWalkSettings call() throws Exception {
                return getXWalkView().getXWalkViewContentForTest().getSettings();
            }
        });
    }

    @SmallTest
    @Feature({"UserAgent"})
    public void testUserAgent() throws Throwable {
        XWalkSettings settings = getXWalkSettingsOnUiThread();
        final String defaultUserAgentString = settings.getUserAgentString();

        // Check that an attempt to reset the default UA string has no effect.
        settings.setUserAgentString(null);
        assertEquals(defaultUserAgentString, settings.getUserAgentString());
        settings.setUserAgentString("");
        assertEquals(defaultUserAgentString, settings.getUserAgentString());

        // Check that we can also set the default value.
        settings.setUserAgentString(defaultUserAgentString);
        assertEquals(defaultUserAgentString, settings.getUserAgentString());

        // Set a custom UA string, verify that it can be reset back to default.
        final String customUserAgentString = "XWalkUserAgentTest";
        settings.setUserAgentString(customUserAgentString);
        assertEquals(customUserAgentString, settings.getUserAgentString());
        settings.setUserAgentString(null);
        assertEquals(defaultUserAgentString, settings.getUserAgentString());
    }
}
