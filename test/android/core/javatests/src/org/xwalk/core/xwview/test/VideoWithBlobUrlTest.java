// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;
import android.util.Pair;

import java.util.List;

import org.chromium.base.test.util.Feature;
import org.chromium.net.test.util.TestWebServer;
import org.xwalk.core.JavascriptInterface;
import org.xwalk.core.xwview.test.util.CommonResources;

/**
 * Test suite for video with blob url.
 */
public class VideoWithBlobUrlTest extends XWalkViewTestBase {
    private TestWebServer mWebServer;
    private boolean mPlaying = false;

    @Override
    public void setUp() throws Exception {
        super.setUp();

        mWebServer = TestWebServer.start();
    }

    @Override
    protected void tearDown() throws Exception {
        mWebServer.shutdown();
        super.tearDown();
    }

    class TestJavascriptInterface {
        @JavascriptInterface
        public void setPlaying() {
            mPlaying = true;
        }
    }

    private void addJavascriptInterface() {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getXWalkView().addJavascriptInterface(new TestJavascriptInterface(),
                        "testInterface");
            }
        });
    }

    private static List<Pair<String, String>> getHTMLHeaders(boolean disableCache) {
        return CommonResources.getContentTypeAndCacheHeaders("text/html", disableCache);
    }

    private String addPageToTestServer(String httpPath, String html) {
        return mWebServer.setResponse(httpPath, html, getHTMLHeaders(false));
    }

    @SmallTest
    @Feature({"video with blob url"})
    public void testVideoPlayWithBlobUrl() throws Throwable {
        String fileContent = getFileContent("save_video_data.html");
        String mainPageUrl = addPageToTestServer("/mainsave",
                CommonResources.makeHtmlPageFrom("", fileContent));

        loadUrlSync(mainPageUrl);
        // wait for writing is done.
        Thread.sleep(1000);

        addJavascriptInterface();
        fileContent = getFileContent("read_video_data.html");
        mainPageUrl = addPageToTestServer("/mainread",
                CommonResources.makeHtmlPageFrom("", fileContent));

        loadUrlSync(mainPageUrl);
        // wait for playing.
        Thread.sleep(1000);
        assertTrue(mPlaying);
    }
}
