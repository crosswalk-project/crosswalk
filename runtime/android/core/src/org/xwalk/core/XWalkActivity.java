// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.os.Bundle;

/**
 * <p><code>XWalkActivity</code> helps to execute all procedures to make the Crosswalk runtime
 * workable and displays dialogs to interact with the user if needed. The activities that hold
 * the {@link XWalkView} objects might want to extend <code>XWalkActivity</code> to obtain this
 * capability. For those activities, there's no need to use {@link XWalkInitializer} and
 * {@link XWalkUpdater}.
 *
 * <p><code>XWalkActivity</code> will try to run in embedded mode firstly. If the Crosswalk runtime
 * hasn't been embedded in the app, or the embedded Crosswalk runtime doesn't match the
 * CPU architecture of the device, it will switch to shared mode, i.e., link to the separately
 * installed Crosswalk runtime package which is shared by multiple apps. If the Crosswalk runtime
 * hasn't been installed on the device, or the installed Crosswalk runtime isn't compatible with
 * the app, it will pop up dialogs to prompt the user to download suitable one. Once the user has
 * agreed to download, it will navigate to the Crosswalk runtime's page on the default application
 * store, subsequent process will be up to the user. If the developer specified the download URL of
 * the Crosswalk runtime, it will launch the download manager to fetch the package. To specify the
 * download URL, insert a meta-data element with the name "xwalk_apk_url" inside the application
 * tag in the Android manifest.
 *
 * <pre>
 * &lt;application&gt;
 *     &lt;meta-data android:name="xwalk_apk_url" android:value="http://host/XWalkRuntimeLib.apk" /&gt;
 * &lt;/&gt;
 * </pre>
 *
 * <p>To download the Crosswalk runtime APK for the specified CPU architecture, <code>xwalk_apk_url<code>
 * will be appended with a query string named "?arch=CPU_ABI" when the download request is sent to server,
 * then server can send back the APK which is exactly built for the specified CPU architecture. The CPU_ABI
 * here is exactly same as the value returned from "getprop ro.product.cpu.abi".</p>
 *
 * <p>Besides the embedded mode and shared mode, silent download mode is also supported. In silent
 * download mode, the Crosswalk runtime will be downloaded in the background silently without any
 * user interaction. To enable silent download mode, you need to insert the meta-data element "xwalk_apk_url"
 * as mentioned above into the AndroidManifest.xml and override <code>XWalkActivity.shouldEnableDownloadMode
 * <code> to get it return true.</p>
 *
 * <p>In embedded mode, the developer can use the embedding API in <code>onCreate()</code> directly.
 * But in shared mode and lite mode, the Crosswalk runtime isn't loaded yet at the moment the
 * activity is created, so the embedding API won't be usable immediately. To make your code
 * compatible with all modes, it's recommended that all routines using the embedding API should be
 * inside {@link #onXWalkReady} or after {@link #onXWalkReady} is invoked. Please refer to following
 * example for more details.</p>
 *
 * <p>For example:</p>
 *
 * <pre>
 * public class MyActivity extends XWalkActivity {
 *     XWalkView mXWalkView;
 *
 *     &#64;Override
 *     protected void onCreate(Bundle savedInstanceState) {
 *         super.onCreate(savedInstanceState);
 *
 *         // Until onXWalkReady() is invoked, you should do nothing with the embedding API
 *         // except the following:
 *         // 1. Instanciate the XWalkView object
 *         // 2. Call XWalkPreferences.setValue()
 *         // 3. Call XWalkView.setUIClient()
 *         // 4. Call XWalkView.setResourceClient()
 *
 *         XWalkPreferences.setValue(XWalkPreferences.ANIMATABLE_XWALK_VIEW, true);
 *
 *         setContentView(R.layout.activity_xwalkview);
 *         mXWalkView = (XWalkView) findViewById(R.id.xwalkview);
 *         mXWalkView.setUIClient(new MyXWalkUIClient(mXWalkView));
 *         mXWalkView.setResourceClient(new MyXWalkResourceClient(mXWalkView));
 *     }
 *
 *     &#64;Override
 *     public void onXWalkReady() {
 *         // Do anyting with the embedding API
 *         mXWalkView.load("http://crosswalk-project.org/", null);
 *     }
 * }
 * </pre>
 *
 * <p>To download the Crosswalk runtime, you need to grant following permissions in the
 * Android manifest:</p>
 *
 * <pre>
 * &lt;uses-permission android:name="android.permission.ACCESS_NETWORK_STATE" /&gt;
 * &lt;uses-permission android:name="android.permission.ACCESS_WIFI_STATE" /&gt;
 * &lt;uses-permission android:name="android.permission.INTERNET" /&gt;
 * &lt;uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" /&gt;
 * </pre>
 */
public abstract class XWalkActivity extends Activity {
    private XWalkActivityDelegate mActivityDelegate;

    /**
     * Run on the UI thread to notify the Crosswalk runtime is ready.
     */
    protected abstract void onXWalkReady();

    /**
     * Override this and return true if you want to get your app work
     * under silent download mode. By default, it returns false.
     */
    protected boolean shouldEnableDownloadMode() {
        return false;
    }

    /**
     * In download mode, this is used to notify the background update progress
     * It runs in UI thread, developer can override it to show update progress UI
     * @param percentage the update progress in percentage
     */
    protected void onXWalkUpdateProgress(int percentage) {}

    /**
     * Return true if the Crosswalk runtime is ready, false otherwise.
     */
    public boolean isXWalkReady() {
        return mActivityDelegate.isXWalkReady();
    }

    /**
     * Return true if running in shared mode, false otherwise.
     */
    public boolean isSharedMode() {
        return mActivityDelegate.isSharedMode();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (shouldEnableDownloadMode()) {
            mActivityDelegate = new XWalkActivityDelegate(this,
                new XWalkActivityDelegate.XWalkUpdateListener() {
                    @Override
                    public void onXWalkUpdateStarted() {}

                    @Override
                    public void onXWalkUpdateProgress(int percentage) {
                        XWalkActivity.this.onXWalkUpdateProgress(percentage);
                    }

                    @Override
                    public void onXWalkUpdateCanceled() {}

                    @Override
                    public void onXWalkUpdateFailed() {}

                    @Override
                    public void onXWalkUpdateCompleted() {
                        onXWalkReady();
                    }
                });
        } else {
            Runnable cancelCommand = new Runnable() {
                @Override
                public void run() {
                    finish();
                }
            };
            Runnable completeCommand = new Runnable() {
                @Override
                public void run() {
                    onXWalkReady();
                }
            };
            mActivityDelegate = new XWalkActivityDelegate(this,
                    cancelCommand, completeCommand);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        mActivityDelegate.onResume();
    }
}
