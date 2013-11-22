// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.test.util;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.test.ActivityInstrumentationTestCase2;

import java.io.IOException;
import java.lang.Process;
import java.lang.Runtime;
import java.lang.StringBuffer;
import java.util.concurrent.atomic.AtomicReference;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.Timer;
import java.util.TimerTask;

/**
 * Test helper for Runtime client APIs.
 */
public class RuntimeClientApiTestBase<T extends Activity> {
    private XWalkRuntimeClientTestUtilBase mTestUtil;
    private ActivityInstrumentationTestCase2<T> mTestCase;
    private Timer mTimer = new Timer();
    private String mSocketName;
    private String mUrl = "http://www.bing.com";
    enum Relation {
        EQUAL,
        GREATERTHAN,
        LESSSTHAN,
        NONE
    }

    public RuntimeClientApiTestBase(XWalkRuntimeClientTestUtilBase testUtil,
            ActivityInstrumentationTestCase2<T> testCase) {
        mTestUtil = testUtil;
        mTestCase = testCase;
    }

    public String getTitleOnUiThread() {
        mTestCase.getInstrumentation().waitForIdleSync();
        final AtomicReference<String> title = new AtomicReference<String>();
        mTestCase.getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                title.set(mTestUtil.getTestedView().getTitleForTest());
            }
        });
        return title.get();
    }

    public void compareTitle(String prevTitle, String title, String msg, Relation relation) {
        if (prevTitle == null) prevTitle = "";
        if (title == null) title = "";

        switch (relation) {
            case EQUAL:
                mTestCase.assertTrue(msg, title.equals(prevTitle));
                break;
            case GREATERTHAN:
                mTestCase.assertTrue(msg, title.compareTo(prevTitle) > 0);
                break;
            default:
                break;
        }
    }

    public void waitForTimerFinish(int timer) throws Throwable {
        Object notify = new Object();
        synchronized (notify) {
            NotifyTask testTask = new NotifyTask(notify);
            mTimer.schedule(testTask, timer);
            notify.wait();
        }
    }

    public class NotifyTask extends TimerTask {
        private Object mObj;

        public NotifyTask(Object obj) {
            super();
            mObj = obj;
        }

        @Override
        public void run() {
            synchronized (mObj) {
                mObj.notify();
            }
        }
    }

    public void sendBroadCast(Activity activity, Context context, String extra) throws Throwable {
        mSocketName = activity.getPackageName();
        Intent intent = new Intent().setAction("org.xwalk.intent").putExtra("remotedebugging", extra);
        context.sendBroadcast(intent);
    }

    public int getSocketNameIndex() {
        try {
            // Try to find the abstract socket name opened for remote debugging
            // from the output of 'cat /proc/net/unix' command. Actually, the
            // best way to test DevToolsServer is to connect the server socket
            // by android.net.LocalSocket and communicate with it (e.g. send
            // http request to query all inspectable pages). However, since
            // the socket of devtools server is enforced to be connected only
            // if the user is authenticated. On a non-rooted device, it only
            // authenticates 'shell' user which is reserved for adb connecction.
            Process process = Runtime.getRuntime().exec("cat /proc/net/unix");

            final int BUFFER_SIZE = 1024;
            byte[] bytes = new byte[BUFFER_SIZE];
            StringBuffer buffer = new StringBuffer(4 * BUFFER_SIZE);

            int bytesReceived = process.getInputStream().read(bytes, 0, BUFFER_SIZE);
            while (bytesReceived > 0) {
                String tmp = new String(bytes, 0, bytesReceived);
                buffer.append(tmp);
                bytesReceived = process.getInputStream().read(bytes, 0, BUFFER_SIZE);
            }

            process.destroy();

            String contents = new String(buffer);
            int index = contents.indexOf(mSocketName + "_devtools_remote");
            return index;
        } catch (IOException e) {
            mTestCase.fail("error occurs in testDevTools: " + e);
            return 0;
        }
    }

    // For loadAppFromUrl.
    public void testLoadAppFromUrl() throws Throwable {
        final String expectedTitle = "Crosswalk Sample Application";

        mTestUtil.loadUrlSync("file:///android_asset/index.html");
        mTestCase.getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                String title = mTestUtil.getTestedView().getTitleForTest();
                mTestCase.assertEquals(expectedTitle, title);
            }
        });
    }

    // For loadAppFromManifest.
    public void testLoadAppFromManifest() throws Throwable {
        final String expectedTitle = "Crosswalk Sample Application";

        mTestUtil.loadManifestSync("file:///android_asset/manifest.json");

        mTestCase.getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                String title = mTestUtil.getTestedView().getTitleForTest();
                mTestCase.assertEquals(expectedTitle, title);
            }
        });
    }

    // For internal extension implementation of DeviceCapabilities.
    public void testDeviceCapabilities() throws Throwable {
        String title = mTestUtil.loadAssetFileAndWaitForTitle("device_capabilities.html");
        mTestCase.assertEquals("Pass", title);
    }

    // For internal extension implementation of Presentation.
    public void testPresentationDisplayAvailable() throws Throwable {
        String title = mTestUtil.loadAssetFileAndWaitForTitle("displayAvailableTest.html");
        if (isSecondaryDisplayAvailable()) {
            mTestCase.assertEquals("Available", title);
        } else {
            mTestCase.assertEquals("Unavailable", title);
        }
    }

    // For external extension mechanism: async mode.
    public void testExternalExtensionAsync() throws Throwable {
        String title = mTestUtil.loadAssetFileAndWaitForTitle("echo.html");
        mTestCase.assertEquals("Pass", title);
    }

    // For external extension mechanism: sync mode.
    public void testExternalExtensionSync() throws Throwable {
        mTestUtil.loadAssetFile("echoSync.html");
        mTestCase.getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                String title = mTestUtil.getTestedView().getTitleForTest();
                mTestCase.assertEquals("Pass", title);
            }
        });
    }

    // For onPause, onResume.
    public void testPauseAndResume() throws Throwable {
        String title = "";
        String prevTitle = "";
        String nextTitle = "";
        String msg = "";

        title = mTestUtil.loadAssetFileAndWaitForTitle("timer.html");
        mTestCase.assertNotNull(title);

        mTestUtil.onPause();
        msg = "The second title should be equal to the first title.";
        // wait for the pause is finished.
        waitForTimerFinish(5000);
        prevTitle = getTitleOnUiThread();
        waitForTimerFinish(200);
        nextTitle = getTitleOnUiThread();
        compareTitle(prevTitle, nextTitle, msg, Relation.EQUAL);

        mTestUtil.onResume();
        msg = "The second title should be greater than the first title.";
        // wait for the resume is finished.
        waitForTimerFinish(5000);
        prevTitle = getTitleOnUiThread();
        waitForTimerFinish(200);
        nextTitle = getTitleOnUiThread();
        compareTitle(prevTitle, nextTitle, msg, Relation.GREATERTHAN);
    }

    // For enable the remote debugging.
    public void testEnableRemoteDebugging(Activity activity, Context context) throws Throwable {
        sendBroadCast(activity, context, "true");
        mTestUtil.loadUrlSync(mUrl);
        int index = getSocketNameIndex();
        mTestCase.assertTrue (index != -1);
        sendBroadCast(activity, context, "false");
    }

    //  For disable the remote debugging.
    public void testDisableRemoteDebugging(Activity activity, Context context) throws Throwable {
        sendBroadCast(activity, context, "false");
        mTestUtil.loadUrlSync(mUrl);
        int index = getSocketNameIndex();
        mTestCase.assertTrue (index < 0);
    }

    // For getVersion.
    public void testGetVersion() throws Throwable {
        String version = mTestUtil.getTestedView().getVersion();

        Pattern pattern = Pattern.compile("\\d+\\.\\d+\\.\\d+\\.\\d+");
        Matcher matcher = pattern.matcher(version);
        mTestCase.assertTrue("The version is invalid.", matcher.find());
    }

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR1)
    private boolean isSecondaryDisplayAvailable() {
        String value;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
            value =  android.provider.Settings.Global.getString(
                             mTestCase.getActivity().getContentResolver(),
                             "overlay_display_devices");
        } else {
            value = null;
        }

        if (value != null && value.length() > 0) {
            return true;
        }
        return false;
    }
}
