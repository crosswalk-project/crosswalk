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
import org.xwalk.core.XWalkClient;
import org.xwalk.core.XWalkGeolocationPermissions;
import org.xwalk.core.XWalkView;
import org.xwalk.core.XWalkWebChromeClient;

/**
 * Test suite for onGeolocationPermissionsShowPrompt() and
 *                onGeolocationPermissionsHidePrompt().
 */
public class GeolocationPermissionTest extends XWalkViewTestBase {
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
                getXWalkView().getSettings().setJavaScriptEnabled(true);
                getXWalkView().getSettings().setGeolocationEnabled(true);
            }
        });
    }

    @SmallTest
    @Feature({"GeolocationPermission"})
    public void testGeolocationPermissionShowPrompt() throws Throwable {
        class TestWebChromeClient extends XWalkWebChromeClient {
            private int mCalledCount = 0;
            @Override
            public void onGeolocationPermissionsShowPrompt(String origin,
                    XWalkGeolocationPermissions.Callback callback) {
                // The origin is empty for data stream.
                assertTrue(origin.isEmpty());
                callback.invoke(origin, true, true);
                mCalledCount++;
            }

            public int getCalledCount() {
                return mCalledCount;
            }
        }
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getXWalkView().setXWalkWebChromeClient(new TestWebChromeClient());
            }
        });
        loadAssetFile("geolocation.html");
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                TestWebChromeClient chromeClient =
                        (TestWebChromeClient)getXWalkView().getXWalkWebChromeClientForTest();
                assertEquals(1, chromeClient.getCalledCount());
            }
        });
    }

    // This is not used now. Need a TODO to follow up this.
    // TODO(hengzhi): how to verify it automaticly.
    // @SmallTest
    // @Feature({"GeolocationPermission"})
    @DisabledTest
    public void testGeolocationPermissionHidePrompt() throws Throwable {
        class TestWebChromeClient extends XWalkWebChromeClient {
            @Override
            public void onGeolocationPermissionsHidePrompt() {
                // Do something.
            }
        }
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getXWalkView().setXWalkWebChromeClient(new TestWebChromeClient());
            }
        });
        loadUrlSync("http://html5demos.com/geo");
    }
}
