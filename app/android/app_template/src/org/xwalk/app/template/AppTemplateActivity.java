// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app.template;

import android.graphics.Color;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.widget.TextView;

import org.xwalk.app.XWalkRuntimeActivityBase;

public class AppTemplateActivity extends XWalkRuntimeActivityBase {
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        // Passdown the key-up event to runtime view.
        if (getRuntimeView() != null &&
                getRuntimeView().onKeyUp(keyCode, event)) {
            return true;
        }

        return super.onKeyUp(keyCode, event);
    }

    @Override
    protected void didTryLoadRuntimeView(View runtimeView) {
        if (runtimeView != null) {
            setContentView(runtimeView);
            getRuntimeView().loadAppFromUrl("file:///android_asset/index.html");
        } else {
            TextView msgText = new TextView(this);
            msgText.setText(R.string.dialog_message_install_runtime_lib);
            msgText.setTextSize(36);
            msgText.setTextColor(Color.BLACK);
            setContentView(msgText);
        }
    }
}
