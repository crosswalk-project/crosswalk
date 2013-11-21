// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.test.suitebuilder.annotation.SmallTest;
import org.chromium.base.test.util.Feature;
import org.xwalk.core.XWalkView;
import org.xwalk.core.XWalkClient;
import org.xwalk.core.client.XWalkDefaultNavigationHandler;

/**
 * Test suite for handling ActionUri.
 */
public class HandleActionUriTest extends XWalkViewTestBase {

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

    class TestXWalkNavigationHandler extends XWalkDefaultNavigationHandler {
        private Intent intentToStart;

        public TestXWalkNavigationHandler(Context context) {
            super(context);
        }

        @Override
        protected boolean startActivity(Intent intent) {
            // For testing purpose, instead of sending out the Intent,
            // just keep it for verification.
            intentToStart = intent;
            return true;
        }

        protected Intent getIntent() {
            return intentToStart;
        }
    }

    private TestXWalkNavigationHandler mNavigationHandler;

    @Override
    public void setUp() throws Exception {
        super.setUp();

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getXWalkView().setXWalkClient(new TestXWalkClient());
                mNavigationHandler = new TestXWalkNavigationHandler(
                        getXWalkView().getActivity());
                getXWalkView().setNavigationHandler(mNavigationHandler);
            }
        });
    }

    /**
     * @param uri the uri to be loaded
     * @return the Intent will be used to start activity.
     * @throws Throwable
     */
    private Intent loadActionUri(String uri) throws Throwable {
        loadUrlSync(uri);
        return mNavigationHandler.getIntent();
    }

    @SmallTest
    @Feature({"ActionUri"})
    public void testTelUri() throws Throwable {
        final String uri = "tel:5551212";
        Intent intent = loadActionUri(uri);
        assertEquals(uri, intent.toUri(0).substring(0, uri.length()));
    }

    @SmallTest
    @Feature({"ActionUri"})
    public void testMailUri() throws Throwable {
        final String uri = "mailto:abc@corp.com";
        Intent intent = loadActionUri(uri);
        assertEquals(uri, intent.toUri(0).substring(0, uri.length()));
    }

    @SmallTest
    @Feature({"ActionUri"})
    public void testSmsUri() throws Throwable {
        final String body = "This is the message";
        final String address = "5551212";
        final String uri = "sms:" + address + "?body=" + body;
        Intent intent = loadActionUri(uri);
        assertEquals("vnd.android-dir/mms-sms", intent.getType());
        assertEquals(body, intent.getStringExtra("sms_body"));
        assertEquals(address, intent.getStringExtra("address"));
    }

    @SmallTest
    @Feature({"ActionUri"})
    public void testGeoUri() throws Throwable {
        final String uri = "geo:0,0?q=address";
        Intent intent = loadActionUri(uri);
        assertEquals(uri, intent.toUri(0).substring(0, uri.length()));
    }

    @SmallTest
    @Feature({"ActionUri"})
    public void testMarketUri() throws Throwable {
        final String uri = "market:";
        Intent intent = loadActionUri(uri);
        assertEquals(uri, intent.toUri(0).substring(0, uri.length()));
    }

    @SmallTest
    @Feature({"WTAIUri"})
    public void testWTAICallUri() throws Throwable {
        final String uri = "wtai://wp/mc;5551212";
        Intent intent = loadActionUri(uri);
        final String equalTelUri = uri.replace("wtai://wp/mc;", "tel:");
        assertEquals(equalTelUri,
                intent.toUri(0).substring(0, equalTelUri.length()));
    }
}
