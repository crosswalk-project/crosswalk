// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.graphics.Bitmap;
import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.Feature;
import org.chromium.net.test.util.TestWebServer;
import org.xwalk.core.XWalkView;
import org.xwalk.core.internal.XWalkClient;

/**
 * Test suite for reload().
 */
public class ReloadTest extends XWalkViewTestBase {

    private TestWebServer mWebServer;

    @Override
    public void setUp() throws Exception {
        super.setUp();

        mWebServer = TestWebServer.start();
    }

    @Override
    public void tearDown() throws Exception {
        mWebServer.shutdown();
        super.tearDown();
    }

    // TODO(guangzhen): Since the device issue, it can not access the network,
    // so disabled this test temporarily. It will be enabled later.
    // @SmallTest
    // @Feature({"reload"})
    @DisabledTest
    public void testReloadUrl() throws Throwable {
        /*
        String title1 = "title1";
        String title2 = "title2";
        String html1 = "<html><head><title>" + title1 + "</title></head></html><body></body>";
        String html2 = "<html><head><title>" + title2 + "</title></head></html><body></body>";
        String url = mWebServer.setResponse("/reload.html", html1, null);
        loadUrlSync(url);
        mWebServer.setResponse("/reload.html", html2, null);
        reloadSync(XWalkView.RELOAD_IGNORE_CACHE);
        //TODO(guangzhen) When reload finished, immediately call getTitle will get wrong title.
        Thread.sleep(1000);
        assertEquals(title2, getTitleOnUiThread());
        */
    }
}
