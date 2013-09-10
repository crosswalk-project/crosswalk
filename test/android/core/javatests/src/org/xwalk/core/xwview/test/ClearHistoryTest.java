// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.graphics.Bitmap;
import android.test.suitebuilder.annotation.SmallTest;
import org.chromium.base.test.util.Feature;
import org.xwalk.core.XWalkClient;
import org.xwalk.core.XWalkView;

/**
 * Test suite for loadUrl().
 */
public class ClearHistoryTest extends XWalkViewTestBase {
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
        getXWalkView().setXWalkClient(new TestXWalkClient());
    }

    @SmallTest
    @Feature({"ClearHistory"})
    public void testClearHistory() throws Throwable {
        final String url1 = "http://www.webkit.org/";
        final String url2 = "http://www.baidu.com/";
        loadUrlSync(url1);
        loadUrlSync(url2);
        clearHistoryOnUiThread();
        assertFalse(canGoBackOnUiThread());
    }
}
