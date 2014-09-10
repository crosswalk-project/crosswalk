// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

/**
 * Test suite for loadAppFromManifest().
 */
public class LoadAppFromManifestTest extends XWalkViewTestBase {
    final String mExpectedTitle = "Crosswalk Sample Application";
    final String mExpectedStr = "The WebKit Open Source Project";

    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @SmallTest
    @Feature({"LoadAppFromManifest"})
    public void testHttpUrl() throws Throwable {
        // TODO(hengzhi): get the valid path for manifest.json.
        final String url = "http://path/manifest.json";

        // loadManifestSync(url, null);
        // assertEquals(mExpectedTitle, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"LoadAppFromManifest"})
    public void testHttpsUrl() throws Throwable {
        // TODO(hengzhi): get the valid path for manifest.json.
        final String url = "https://path/manifest.json";

        // loadManifestSync(url, null);
        // assertEquals(mExpectedTitle, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"LoadAppFromManifest"})
    public void testAndroidAssetUrl() throws Throwable {
        final String url = "file:///android_asset/www/manifest.json";

        loadManifestSync(url, null);
        assertEquals(mExpectedTitle, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"LoadAppFromManifest"})
    public void testWithData() throws Throwable {
        final String url = "file:///android_asset/www/";
        String name = "manifest.json";
        final String finalContent = getFileContent(name);
        String content;

        loadManifestSync(url, finalContent);
        assertEquals(mExpectedTitle, getTitleOnUiThread());

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                try {
                    getXWalkView().loadAppFromManifest(null, finalContent);
                    assertEquals(mExpectedTitle, getTitleOnUiThread());
                } catch (Exception e) {
                    assertTrue(e instanceof RuntimeException);
                }
            }
        });

        name = "manifest_with_web_url.json";
        content = getFileContent(name);
        loadManifestSync(url, content);
        assertEquals(mExpectedStr, getTitleOnUiThread());

        name = "manifest_with_local_url.json";
        content = getFileContent(name);
        loadManifestSync(url, content);
        assertEquals(mExpectedTitle, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"LoadAppFromManifest"})
    public void testWithEmptyUrlAndContent() throws Throwable {
        loadManifestAsync(null, null);
        Thread.sleep(1000);
        assertNotNull(getTitleOnUiThread());
    }
}
