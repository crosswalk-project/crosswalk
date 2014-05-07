// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

/**
 * Test suite for load().
 */
public class LoadTest extends XWalkViewTestBase {
    final String expectedTitle = "The WebKit Open Source Project";
    final String expectedLocalTitle = "Crosswalk Sample Application";

    @Override
    public void setUp() throws Exception {
        super.setUp();

        setXWalkClient(new XWalkViewTestBase.TestXWalkClient());
    }

    @SmallTest
    @Feature({"Load"})
    public void testHttpUrl() throws Throwable {
        final String url = "http://www.webkit.org/";

        loadUrlSync(url);
        assertEquals(expectedTitle, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"Load"})
    public void testHttpsUrl() throws Throwable {
        final String url = "https://www.webkit.org/";

        loadUrlSync(url);
        assertEquals(expectedTitle, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"Load"})
    public void testAndroidAssetUrl() throws Throwable {
        final String url = "file:///android_asset/www/index.html";

        loadUrlSync(url);
        assertEquals(expectedLocalTitle, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"LoadWithData"})
    public void testWithData() throws Throwable {
        final String name = "index.html";
        String fileContent = getFileContent(name);

        loadDataSync(null, fileContent, "text/html", false);
        assertEquals(expectedLocalTitle, getTitleOnUiThread());

        loadDataSync(name, fileContent, "text/html", false);
        assertEquals(expectedLocalTitle, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"ContentScheme"})
    public void testContentUrl() throws Throwable {
        final String resource = "content_test";
        final String contentUrl = TestContentProvider.createContentUrl(resource);

        int count =
                TestContentProvider.getResourceRequestCount(getActivity(), resource);
        loadUrlSync(contentUrl);
        assertEquals(count + 1,
                TestContentProvider.getResourceRequestCount(getActivity(), resource));
    }

    @SmallTest
    @Feature({"Load"})
    public void testEmpytUrlAndContent() throws Throwable {
        loadDataAsync(null, null, "text/html", false);
        Thread.sleep(1000);
        assertNotNull(getTitleOnUiThread());
    }
}
