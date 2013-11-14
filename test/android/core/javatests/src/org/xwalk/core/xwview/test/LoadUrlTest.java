// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.graphics.Bitmap;
import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;
import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.Feature;
import org.xwalk.core.XWalkView;
import org.xwalk.core.XWalkClient;

/**
 * Test suite for loadUrl().
 */
public class LoadUrlTest extends XWalkViewTestBase {
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

    // @SmallTest
    // @Feature({"LoadUrl"})
    // TODO(hengzhi): Since the device issue, it can not access the network,
    // so disabled this test temporarily. It will be enabled later.
    @DisabledTest
    public void testNormalUrl() throws Throwable {
        final String url = "http://www.webkit.org/";
        final String expected_title = "The WebKit Open Source Project";
        loadUrlSync(url);
        assertEquals(expected_title, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"LoadUrl"})
    public void testLocalUrl() throws Throwable {
        final String name = "index.html";
        final String expected_title = "Crosswalk Sample Application";

        loadAssetFile(name);
        assertEquals(expected_title, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"ContentScheme"})
    public void testContentUrl() throws Throwable {
        final String resource = "content_test";
        final String content_url = TestContentProvider.createContentUrl(resource);

        int count_before_load =
                TestContentProvider.getResourceRequestCount(getActivity(), resource);
        loadUrlSync(content_url);
        assertEquals(count_before_load + 1,
                TestContentProvider.getResourceRequestCount(getActivity(), resource));
    } 
}
