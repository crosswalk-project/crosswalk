// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.os.Bundle;

/**
 * <p><code>XWalkActivity</code> helps to execute all procedures for makeing the Crosswalk runtime
 * workable and displays dialogs to interact with the user if necessary. The activities that hold
 * the {@link XWalkView} object might want to extend <code>XWalkActivity</code> to obtain this
 * capability. For those activities, it's important to override the abstract method
 * {@link #onXWalkReady} that notifies the Crosswalk runtime is ready.<p>
 *
 * <p>In shared mode, the Crosswalk runtime is not loaded yet at the moment the activity is created.
 * So the developer can't use the embedding API in <code>onCreate()</code> as usual. All routines
 * using the embedding API should be inside {@link #onXWalkReady} or after {@link #onXWalkReady}
 * is invoked.</p>
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
 *         // Until onXWalkReady() is invoked, you can do nothing with the embedding API
 *         // except the following:
 *         // 1. Create the instance of XWalkView
 *         // 2. Call setUIClient()
 *         // 3. Call setResourceClient()
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
 *
 *         XWalkPreferences.setValue(XWalkPreferences.SUPPORT_MULTIPLE_WINDOWS, true);
 *
 *         mXWalkView.load("http://crosswalk-project.org/", null);
 *     }
 * }
 * </pre>
 *
 * <p>Besides, shared mode needs following permissions:</p>
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
    protected boolean isXWalkReady() {
        return mActivityDelegate.isXWalkReady();
    }

    /**
     * Return true if running in shared mode, false otherwise.
     */
    protected boolean isSharedMode() {
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
