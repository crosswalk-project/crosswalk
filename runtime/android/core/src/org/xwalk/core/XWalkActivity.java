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
 * &lt;application android:name="org.xwalk.core.XWalkApplication"&gt;
 *     &lt;meta-data android:name="xwalk_apk_url" android:value="http://host/XWalkRuntimeLib.apk" /&gt;
 * </pre>
 *
 * <p>In old versions, the developer can use the embedding API in <code>onCreate()</code> directly
 * or any where at any time as they wish. But in latest version, the Crosswalk runtime isn't loaded
 * yet at the moment the activity is created, so the embedding API won't be usable immediately.
 * To make your code compatible with new implementation somtimes, all routines using the embedding
 * API should be inside {@link #onXWalkReady} or after {@link #onXWalkReady} is invoked. Please
 * refer to following example for more details.</p>
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
