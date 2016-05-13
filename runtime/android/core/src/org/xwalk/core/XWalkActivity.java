// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.os.Bundle;

/**
 * <p><code>XWalkActivity</code> helps to execute all procedures to make Crosswalk Project runtime
 * workable and displays dialogs to interact with the user. The activities that hold the
 * {@link XWalkView} objects might want to extend <code>XWalkActivity</code> to obtain this
 * capability. For those activities, there's no need to use {@link XWalkInitializer} and
 * {@link XWalkUpdater}.</p>
 *
 * <p>By <code>XWalkActivity</code>, your application can support all of embedded mode, shared mode
 * and download mode with same code. So this is the preferred interface. </p>
 *
 * <h3>Sample Code</h3>
 *
 * <p>Here is the sample code for all running modes:</p>
 *
 * <pre>
 * import android.os.Bundle;
 * import org.xwalk.core.XWalkActivity;
 * import org.xwalk.core.XWalkView;
 *
 * public class MainActivity extends XWalkActivity {
 *     XWalkView mXWalkView;
 *
 *     &#64;Override
 *     protected void onCreate(Bundle savedInstanceState) {
 *         super.onCreate(savedInstanceState);
 *
 *         // Until onXWalkReady() is invoked, you should do nothing with the
 *         // embedding API except the following:
 *         // 1. Instantiate the XWalkView object
 *         // 2. Call XWalkPreferences.setValue()
 *         // 3. Call mXWalkView.setXXClient(), e.g., setUIClient
 *         // 4. Call mXWalkView.setXXListener(), e.g., setDownloadListener
 *         // 5. Call mXWalkView.addJavascriptInterface()
 *         setContentView(R.layout.activity_main);
 *         mXWalkView = (XWalkView) findViewById(R.id.xwalkview);
 *     }
 *
 *     &#64;Override
 *     public void onXWalkReady() {
 *         // Do anyting with the embedding API
 *         mXWalkView.load("https://crosswalk-project.org/", null);
 *     }
 * }
 * </pre>
 *
 * <h3>App Manifest</h3>
 *
 * <p>For shared mode and download mode, you might need to edit the Android manifest to set some
 * properties. </p>
 *
 * <h5>Shared Mode</h5>
 *
 * <p>If you want the end-user to download Crosswalk Project runtime from specified URL instead of
 * switching to the application store, add following &lt;meta-data&gt; element inside the
 * &lt;application&gt; element:</p>
 *
 * <pre>
 * &lt;application&gt;
 *     &lt;meta-data android:name="xwalk_apk_url" android:value="http://host/XWalkRuntimeLib.apk" /&gt;
 * </pre>
 *
 * <p>Please note that when the HTTP request is sent to server, the URL will be appended with
 * "?arch=CPU_API" to indicate that on which CPU architecture it's currently running. The CPU_API
 * is the same as the value returned from "adb shell getprop ro.product.cpu_abi", e.g. x86 for
 * IA 32bit, x86_64 for IA 64bit, armeabi-v7a for ARM 32bit and arm64-v8a for ARM 64bit.
 *
 * <p>The specified APK will be downloaded to SD card, so you have to grant following permission: </p>
 *
 * <pre>
 * &lt;uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" /&gt;
 * </pre>
 *
 * <h5>Download Mode</h5>
 *
 * <p>Firstly, you need to add following &lt;meta-data&gt; element to enable download mode:</p>
 *
 * <pre>
 * &lt;application&gt;
 *     &lt;meta-data android:name="xwalk_download_mode" android:value="enable"/&gt;
 * </pre>
 *
 * <p>In download mode, the value of <code>xwalk_apk_url</code> is mandatory. However, the
 * downloaded Apk will be saved into application's private storage, so the permission of writing to
 * SD card is not needed anymore.</p>
 *
 * <p>By default, the application will verify the signature of downloaded Crosswalk Project runtime,
 * which is required to be the same as your application. But you can disable it by adding following
 * &lt;meta-data&gt; element:
 *
 * <pre>
 * &lt;application&gt;
 *     &lt;meta-data android:name="xwalk_verify" android:value="disable"/&gt;
 * </pre>
 *
 * <p> If your application has already downloaded Crosswalk Project runtime but the application got
 * an update after that, the build version of shared library you used to bundle with your
 * new application may be newer than the build version of downloaded Crosswalk Project runtime.
 * In this case, it will download new version of Crosswalk Project runtime from the server again.
 * If you want to continue using old version of Crosswalk Project runtime, you could add following
 * &lt;meta-data&gt; element:
 *
 * <pre>
 * &lt;application&gt;
 *     &lt;meta-data android:name="xwalk_download_mode_update" android:value="disable"/&gt;
 * </pre>
 *
 */
public abstract class XWalkActivity extends Activity {
    private XWalkActivityDelegate mActivityDelegate;

    /**
     * Run on the UI thread to notify the Crosswalk runtime is ready.
     */
    protected abstract void onXWalkReady();

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
        mActivityDelegate = new XWalkActivityDelegate(this, cancelCommand, completeCommand);
    }

    @Override
    protected void onResume() {
        super.onResume();
        mActivityDelegate.onResume();
    }
}
