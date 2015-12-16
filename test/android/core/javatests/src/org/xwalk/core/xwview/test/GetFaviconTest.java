// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Message;
import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;

import java.io.InputStream;
import java.net.URL;
import java.util.concurrent.Callable;

import org.chromium.base.test.util.Feature;
import org.chromium.net.test.util.TestWebServer;

import org.xwalk.core.XWalkUIClient;
import org.xwalk.core.XWalkView;
import org.xwalk.core.xwview.test.util.CommonResources;

/**
 * Test suite for getFavicon().
 */
public class GetFaviconTest extends XWalkViewTestBase {
    private TestWebServer webServer;
    private XWalkView mXWalkView;

    @Override
    public void setUp() throws Exception {
        super.setUp();

        webServer = TestWebServer.start();
        mXWalkView = getXWalkView();
        setUIClient(new XWalkUIClient(mXWalkView) {
            /**
             * Must override this function for correct behaviour
             */
            @Override
            public void onIconAvailable(XWalkView view, String url, Message msg) {
                msg.sendToTarget();
            }
        });
    }

    @SmallTest
    @Feature({"getFavicon"})
    public void testGetFavicon() throws Throwable {
        try {
            final String faviconUrl = webServer.setResponseBase64(
                    "/" + CommonResources.FAVICON_FILENAME, CommonResources.FAVICON_DATA_BASE64,
                    CommonResources.getImagePngHeaders(false));
            final String pageUrl = webServer.setResponse("/favicon.html",
                    CommonResources.FAVICON_STATIC_HTML, null);

            loadUrlAsync(pageUrl);

            // The getFavicon will return the right icon a certain time after
            // the page load completes which makes it slightly hard to test.
            pollOnUiThread(new Callable<Boolean>() {
                @Override
                public Boolean call() throws Exception{
                    return mXWalkView.getFavicon() != null;
                }
            });

            final Object originalFaviconSource = (new URL(faviconUrl)).getContent();
            final Bitmap originalFavicon =
                    BitmapFactory.decodeStream((InputStream) originalFaviconSource);

            final Bitmap currentFavicon = getFaviconOnUiThread(); 

            assertNotNull(originalFavicon);

            assertTrue(currentFavicon.sameAs(originalFavicon));
        } finally {
            webServer.shutdown();
        }
    }
}
