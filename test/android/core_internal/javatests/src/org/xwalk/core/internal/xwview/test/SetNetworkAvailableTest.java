// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.xwview.test;

import android.graphics.Bitmap;
import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;
import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.Feature;

import org.xwalk.core.internal.XWalkClient;
import org.xwalk.core.internal.XWalkViewInternal;

/**
 * Test case for XWalkViewInternal.setNetworkAvailable method
 *
 * Once setNetworkAvailable is called, the navigator.onLine property will be
 * set, and window.ononline/onoffline event will be fired if the property is
 * changed.
 */
public class SetNetworkAvailableTest extends XWalkViewInternalTestBase {
    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @Feature({"SetNetworkAvailableTest"})
    @SmallTest
    public void testSetNetworkAvailableTest() throws Throwable {
        final String code = "navigator.onLine";
        loadAssetFile("navigator.online.html");
        String title = getTitleOnUiThread();

        final XWalkViewInternal xwView = getXWalkView();

        if ("true".equals(title)) {
            getInstrumentation().runOnMainSync(new Runnable() {
                @Override
                public void run() {
                    // Forcing to trigger 'offline' event.
                    xwView.setNetworkAvailable(false);
                }
            });

            /**
             * Expectations:
             * 1. navigator.onLine is false;
             * 2. window.onoffline event is fired.
             */
            assertEquals("false", executeJavaScriptAndWaitForResult(code));
            assertEquals("offline:false", getTitleOnUiThread());

            getInstrumentation().runOnMainSync(new Runnable() {
                @Override
                public void run() {
                    // Forcing to trigger 'online' event.
                    xwView.setNetworkAvailable(true);
                }
            });

            /**
             * Expectations:
             * 1. navigator.onLine is true;
             * 2. window.ononline event is fired.
             */
            assertEquals("true", executeJavaScriptAndWaitForResult(code));
            assertEquals("online:true", getTitleOnUiThread());
        }

        if ("false".equals(title)) {
             getInstrumentation().runOnMainSync(new Runnable() {
                 @Override
                 public void run() {
                     // Forcing to trigger 'online' event.
                     xwView.setNetworkAvailable(true);
                 }
             });

            /**
             * Expectations:
             * 1. navigator.onLine is true;
             * 2. window.ononline event is fired.
             */
            assertEquals("true", executeJavaScriptAndWaitForResult(code));
            assertEquals("online:true", getTitleOnUiThread());

            getInstrumentation().runOnMainSync(new Runnable() {
                @Override
                public void run() {
                    // Forcing to trigger 'offline' event.
                    xwView.setNetworkAvailable(false);
                }
            });

            /**
             * Expectations:
             * 1. navigator.onLine is false;
             * 2. window.onoffline event is fired.
             */
            assertEquals("false", executeJavaScriptAndWaitForResult(code));
            assertEquals("offline:false", getTitleOnUiThread());
        }
    }
}
