// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import org.xwalk.core.XWalkView;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageButton;

public class PauseTimersActivity extends Activity {

    private ImageButton mButton;
    private boolean isPaused;
    XWalkView mXWalkView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.pause_timers_layout);
        mXWalkView = (XWalkView) findViewById(R.id.xwalkview);

        isPaused = false;
        mButton = (ImageButton) findViewById(R.id.pause);
        mButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mXWalkView != null) {
                    if (!isPaused) {
                        // Pause JS timer
                        mXWalkView.pauseTimers();
                        isPaused = true;
                        mButton.setImageResource(android.R.drawable.ic_media_play);
                    } else {
                        // Resume JS timer
                        mXWalkView.resumeTimers();
                        isPaused = false;
                        mButton.setImageResource(android.R.drawable.ic_media_pause);
                    }
                }
            }
        });
        mXWalkView.load("file:///android_asset/pause_timers.html", null);
    }
}
