// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;
import android.util.Pair;

import org.chromium.base.test.util.Feature;
import org.chromium.net.test.util.TestWebServer;

import java.util.ArrayList;
import java.util.List;

/**
 * Test suite for clearCache().
 */
public class ClearCacheTest extends XWalkViewTestBase {

    private TestWebServer mWebServer;

    @Override
    public void setUp() throws Exception {
        super.setUp();

        setXWalkClient(new XWalkViewTestBase.TestXWalkClient());
        mWebServer = new TestWebServer(false);
    }

    @Override
    public void tearDown() throws Exception {
        if (mWebServer != null) {
            mWebServer.shutdown();
        }
        super.tearDown();
    }

    @SmallTest
    @Feature({"clearCache"})
    public void testClearCache() throws Throwable {
        final String pagePath = "/clear_cache_test.html";
        List<Pair<String, String>> headers = new ArrayList<Pair<String, String>>();
        // Set Cache-Control headers to cache this request. One century should be long enough.
        headers.add(Pair.create("Cache-Control", "max-age=3153600000"));
        headers.add(Pair.create("Last-Modified", "Mon, 12 May 2014 00:00:00 GMT"));
        final String pageUrl = mWebServer.setResponse(
                pagePath, "<html><body>foo</body></html>", headers);

        // First load to populate cache.
        clearCacheOnUiThread(true);
        loadUrlSync(pageUrl);
        assertEquals(1, mWebServer.getRequestCount(pagePath));

        // Load about:blank so next load is not treated as reload by XWalkView and force
        // revalidate with the server.
        loadUrlSync("about:blank");

        // No clearCache call, so should be loaded from cache.
        loadUrlSync(pageUrl);
        assertEquals(1, mWebServer.getRequestCount(pagePath));

        // Same as above.
        loadUrlSync("about:blank");

        // Clear cache, so should hit server again.
        clearCacheOnUiThread(true);
        loadUrlSync(pageUrl);
        assertEquals(2, mWebServer.getRequestCount(pagePath));

        // Same as above.
        loadUrlSync("about:blank");

        // Do not clear cache, so should be loaded from cache.
        clearCacheOnUiThread(false);
        loadUrlSync(pageUrl);
        assertEquals(2, mWebServer.getRequestCount(pagePath));
    }
}
