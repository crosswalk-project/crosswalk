// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app.template;

import android.graphics.Color;
import android.os.Build.VERSION;
import android.os.Build.VERSION_CODES;
import android.os.Bundle;
import android.view.WindowManager;
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
            getRuntimeView().loadAppFromUrl("file:///android_asset/www/index.html");
        } else {
            TextView msgText = new TextView(this);
            msgText.setText("Crosswalk failed to initialize.");
            msgText.setTextSize(36);
            msgText.setTextColor(Color.BLACK);
            setContentView(msgText);
        }
    }

    private void enterFullscreen() {
        if (VERSION.SDK_INT >= VERSION_CODES.KITKAT &&
                ((getWindow().getAttributes().flags &
                        WindowManager.LayoutParams.FLAG_FULLSCREEN) != 0)) {
            View decorView = getWindow().getDecorView();
            decorView.setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
                    View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
                    View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
                    View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
                    View.SYSTEM_UI_FLAG_FULLSCREEN |
                    View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
        }
    }

    public void setIsFullscreen(boolean isFullscreen) {
        if (isFullscreen) {
            enterFullscreen();
        }
    }

}
