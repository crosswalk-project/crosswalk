// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.client.embedded.test;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.widget.LinearLayout;

import org.xwalk.app.runtime.XWalkRuntimeClient;

/*
 * This is a lightweight activity for tests that only require XWalk functionality.
 */
public class XWalkRuntimeClientTestRunnerActivity extends Activity {
    private LinearLayout mLinearLayout;
    private BroadcastReceiver mReceiver;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mLinearLayout = new LinearLayout(this);
        mLinearLayout.setOrientation(LinearLayout.VERTICAL);
        mLinearLayout.setShowDividers(LinearLayout.SHOW_DIVIDER_MIDDLE);
        mLinearLayout.setLayoutParams(new LayoutParams(LayoutParams.WRAP_CONTENT,
                LayoutParams.WRAP_CONTENT));
        setContentView(mLinearLayout);
    }

    /**
     * Adds a view to the main linear layout.
     */
    public void addView(View view) {
        view.setLayoutParams(new LinearLayout.LayoutParams(
                LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT, 1f));
        mLinearLayout.addView(view);
    }

    /**
     * Clears the main linear layout.
     */
    public void removeAllViews() {
        mLinearLayout.removeAllViews();
    }

    public void registerBroadcastReceiver(final XWalkRuntimeClient runtimeView) {
        IntentFilter intentFilter = new IntentFilter("org.xwalk.intent");
        mReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                Bundle bundle = intent.getExtras();
                if (bundle == null) return;

                if (bundle.containsKey("remotedebugging")) {
                    String extra = bundle.getString("remotedebugging");
                    if (extra.equals("true")) {
                        String mPackageName = getApplicationContext().getPackageName();
                        runtimeView.enableRemoteDebugging("", mPackageName);
                    } else if (extra.equals("false")) {
                        runtimeView.disableRemoteDebugging();
                    }
                }
            }
        };
        registerReceiver(mReceiver, intentFilter);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mReceiver);
    }
}
