// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app;

import android.app.Activity;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

import org.xwalk.app.runtime.XWalkRuntimeView;
import org.xwalk.core.XWalkActivity;

public abstract class XWalkRuntimeActivityBase extends XWalkActivity {
    private static final String TAG = "XWalkRuntimeActivity";

    private static final int SYSTEM_UI_OPTIONS =
            View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
            View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
            View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
            View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
            View.SYSTEM_UI_FLAG_FULLSCREEN |
            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;

    private XWalkRuntimeView mRuntimeView;
    private boolean mRemoteDebugging;
    private boolean mUseAnimatableView;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mRuntimeView = new XWalkRuntimeView(this, null);
        setContentView(mRuntimeView);

        mRuntimeView.setRemoteDebugging(mRemoteDebugging);
        mRuntimeView.setUseAnimatableView(mUseAnimatableView);
    }

    @Override
    protected void onXWalkFailed() {
        didTryLoadRuntimeView(null);
    }

    @Override
    protected void onXWalkReady() {
        mRuntimeView.onCreate();
        didTryLoadRuntimeView(mRuntimeView);
    }

    @Override
    public void onStart() {
        super.onStart();
        mRuntimeView.onStart();
    }

    @Override
    public void onResume() {
        super.onResume();
        mRuntimeView.onResume();
        enterFullscreen();
    }

    @Override
    public void onPause() {
        super.onPause();
        mRuntimeView.onPause();
    }

    @Override
    public void onStop() {
        super.onStop();
        mRuntimeView.onStop();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mRuntimeView.onDestroy();
    }

    @Override
    public void onNewIntent(Intent intent) {
        if (!mRuntimeView.onNewIntent(intent)) super.onNewIntent(intent);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        mRuntimeView.onActivityResult(requestCode, resultCode, data);
    }

    public XWalkRuntimeView getRuntimeView() {
        return mRuntimeView;
    }

    /*
     * Called each time trying to load runtime view from library apk.
     * Descendant should handle both succeeded and failed to load
     * library apk.
     *
     * @param, The RuntimeView loaded, it can be null for failed to load RuntimeView.
     */
    abstract protected void didTryLoadRuntimeView(View runtimeView);

    protected void setRemoteDebugging(boolean value) {
        Log.d(TAG, "Set remote debugging to " + value);
        mRemoteDebugging = value;
    }

    protected void setUseAnimatableView(boolean value) {
        Log.d(TAG, "Set use animatable view to " + value);
        mUseAnimatableView = value;
    }

    protected void setIsFullscreen(boolean isFullscreen) {
        if (!isFullscreen || Build.VERSION.SDK_INT < Build.VERSION_CODES.KITKAT) {
            return;
        }

        Log.d(TAG, "Set full screen");
        final View decorView = getWindow().getDecorView();
        decorView.setOnSystemUiVisibilityChangeListener(
                new View.OnSystemUiVisibilityChangeListener() {
                    @Override
                    public void onSystemUiVisibilityChange(int visibility) {
                        if ((visibility & View.SYSTEM_UI_FLAG_FULLSCREEN) == 0) {
                            decorView.setSystemUiVisibility(SYSTEM_UI_OPTIONS);
                        }
                    }
                }
        );
    }

    private void enterFullscreen() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.KITKAT) {
            return;
        }

        if ((getWindow().getAttributes().flags & WindowManager.LayoutParams.FLAG_FULLSCREEN) != 0) {
            Log.d(TAG, "Enter full screen");
            getWindow().getDecorView().setSystemUiVisibility(SYSTEM_UI_OPTIONS);
        }
    }
}
