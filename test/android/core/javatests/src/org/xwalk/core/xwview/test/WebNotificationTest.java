// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import java.lang.Thread;
import java.lang.InterruptedException;

import android.app.Notification;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;

import org.chromium.base.test.util.Feature;
import org.xwalk.core.XWalkClient;
import org.xwalk.core.XWalkView;
import org.xwalk.core.XWalkWebChromeClient;
import org.xwalk.core.client.XWalkDefaultNotificationService;

/**
 * Test suite for web notification API.
 * This test will only cover notification.show() and notification.close().
 * The event handler will be covered in runtime level test. Because that
 * will need activity to participate. 
 */
public class WebNotificationTest extends XWalkViewTestBase {
    class TestXWalkNotificationService extends XWalkDefaultNotificationService {
        private Notification mNotification;

        public TestXWalkNotificationService(Context context, XWalkView view) {
            super(context, view);
        }

        @Override
        public void doShowNotification(int id, Notification notification) {
            // For testing purpose, instead of sending out the notification,
            // just keep the notification.
            mNotification = notification;
        }

        protected void mockClick() {
            if (mNotification != null && mNotification.contentIntent != null) {
                try {
                    mNotification.contentIntent.send();
            	} catch (android.app.PendingIntent.CanceledException e) {}
            }
        }

        protected void mockClose() {
            if (mNotification != null && mNotification.deleteIntent != null) {
                try {
                    mNotification.deleteIntent.send();
            	} catch (android.app.PendingIntent.CanceledException e) {}
            }
        }
    }

    private TestXWalkNotificationService mNotificationService;

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
                mNotificationService = new TestXWalkNotificationService(
                        getXWalkView().getActivity(), getXWalkView());
                getXWalkView().setNotificationService(mNotificationService);
                getXWalkView().enableRemoteDebugging();
            }
        });
    }

    @SmallTest
    @Feature({"WebNotification"})
    public void testWebNotificationShowAndCloseByJs() throws Throwable {
        loadAssetFile("notification.html");
        getInstrumentation().waitForIdleSync();
        executeJavaScriptAndWaitForResult("notify();");
        // Android Notification is sent in another thread,
        // Sleep five seconds to let the notification sending/closing
        // happen first, and then wait for the idle sync which means
        // javascript callback is executed.
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {}
        getInstrumentation().waitForIdleSync();
        assertEquals("notification shown", getTitleOnUiThread());
        executeJavaScriptAndWaitForResult("dismiss();");
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {}
        getInstrumentation().waitForIdleSync();
        assertEquals("notification closed", getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"WebNotification"})
    public void testWebNotificationShowAndCloseByUser() throws Throwable {
        loadAssetFile("notification.html");
        getInstrumentation().waitForIdleSync();
        executeJavaScriptAndWaitForResult("notify();");
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {}
        getInstrumentation().waitForIdleSync();
        assertEquals("notification shown", getTitleOnUiThread());
        mNotificationService.mockClose();
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {}
	getInstrumentation().waitForIdleSync();
        assertEquals("notification closed", getTitleOnUiThread());
    }
}
