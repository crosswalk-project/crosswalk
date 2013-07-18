// Copyright (c) 2012 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package com.intel.xwalk;

import android.app.Activity;
import android.os.Bundle;
import android.webkit.WebView;
import android.util.Log;

import org.chromium.base.ChromiumActivity;
import org.chromium.content.app.LibraryLoader;
import org.chromium.content.browser.AndroidBrowserProcess;
import org.chromium.content.browser.DeviceUtils;
import org.chromium.content.common.CommandLine;
import org.chromium.content.common.ProcessInitException;
import org.chromium.ui.WindowAndroid;

public class XWalkActivity extends ChromiumActivity {
    private final String TAG = XWalkActivity.class.getName();

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (!CommandLine.isInitialized()) {
            CommandLine.init(new String[] {});
        }

        // specify if the UI should be tablet or mobile
        DeviceUtils.addDeviceSpecificUserAgentSwitch(this);

        try {
            LibraryLoader.ensureInitialized(); // load xwalk
            setContentView(R.layout.main);

            XWalkView xwalkView = (XWalkView) findViewById(R.id.xwalkView);
            WindowAndroid windowAndroid = new WindowAndroid(this);
            windowAndroid.restoreInstanceState(savedInstanceState);
            xwalkView.setWindow(windowAndroid);

            AndroidBrowserProcess.init(this, 1); // calls ContentMain

            xwalkView.loadUrl("http://www.google.com");
        } catch (ProcessInitException e) {
            Log.e(TAG, "Initialization failed.", e);
            finish();
        }
    }
}

