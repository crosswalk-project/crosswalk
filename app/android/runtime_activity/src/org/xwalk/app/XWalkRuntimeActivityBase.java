// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

import org.xwalk.app.runtime.extension.XWalkRuntimeExtensionManager;
import org.xwalk.app.runtime.XWalkRuntimeView;
import org.xwalk.core.XWalkActivity;
import org.xwalk.core.XWalkPreferences;

public abstract class XWalkRuntimeActivityBase extends XWalkActivity {
    private static final String TAG = "XWalkRuntimeActivityBase";

    private XWalkRuntimeView mRuntimeView;

    private boolean mRemoteDebugging = false;

    private boolean mUseAnimatableView = false;

    private XWalkRuntimeExtensionManager mExtensionManager;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public void onXWalkReady() {
        tryLoadRuntimeView();
        // TODO(sunlin): In shared mode, mRuntimeView.onCreate() will be
        // called after onResume(). Currently, there is no impact because
        // both of onCreate and onResume in XWalkRuntimeView is empty.
        if (mRuntimeView != null) mRuntimeView.onCreate();
    }

    @Override
    public void onStart() {
        super.onStart();
        if (mExtensionManager != null) mExtensionManager.onStart();
    }

    @Override
    public void onPause() {
        super.onPause();
        if (mRuntimeView != null) mRuntimeView.onPause();
        if (mExtensionManager != null) mExtensionManager.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
        if (mRuntimeView != null) mRuntimeView.onResume();
        if (mExtensionManager != null) mExtensionManager.onResume();
    }

    @Override
    public void onStop() {
        super.onStop();
        if (mExtensionManager != null) mExtensionManager.onStop();
    }

    @Override
    public void onDestroy() {
        if (mExtensionManager != null) mExtensionManager.onDestroy();
        super.onDestroy();
    }

    @Override
    public void onNewIntent(Intent intent) {
        if (mRuntimeView == null || !mRuntimeView.onNewIntent(intent)) super.onNewIntent(intent);
        if (mExtensionManager != null) mExtensionManager.onNewIntent(intent);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (mRuntimeView != null) mRuntimeView.onActivityResult(requestCode, resultCode, data);
        if (mExtensionManager != null) mExtensionManager.onActivityResult(requestCode, resultCode, data);
    }

    private void tryLoadRuntimeView() {
        try {
            if (mUseAnimatableView) {
                XWalkPreferences.setValue(XWalkPreferences.ANIMATABLE_XWALK_VIEW, true);
            } else {
                XWalkPreferences.setValue(XWalkPreferences.ANIMATABLE_XWALK_VIEW, false);
            }
            mRuntimeView = new XWalkRuntimeView(this, this, null);
            if (mRemoteDebugging) {
                XWalkPreferences.setValue(XWalkPreferences.REMOTE_DEBUGGING, true);
            } else {
                XWalkPreferences.setValue(XWalkPreferences.REMOTE_DEBUGGING, false);
            }
            // XWalkPreferences.ENABLE_EXTENSIONS
            if (XWalkPreferences.getValue("enable-extensions")) {
                // Enable xwalk extension mechanism and start load extensions here.
                // Note that it has to be after above initialization.
                mExtensionManager = new XWalkRuntimeExtensionManager(getApplicationContext(), this);
                mExtensionManager.loadExtensions();
            }
        } catch (Exception e) {
            handleException(e);
        }
        didTryLoadRuntimeView(mRuntimeView);
    }

    public XWalkRuntimeView getRuntimeView() {
        return mRuntimeView;
    }

    public void handleException(Throwable e) {
        if (e == null) return;
        if (e instanceof RuntimeException && e.getCause() != null) {
            handleException(e.getCause());
            return;
        }
        Log.e(TAG, Log.getStackTraceString(e));
    }

    /*
     * Called each time trying to load runtime view from library apk.
     * Descendant should handle both succeeded and failed to load
     * library apk.
     *
     * @param, The RuntimeView loaded, it can be null for failed to load RuntimeView.
     */
    abstract protected void didTryLoadRuntimeView(View runtimeView);

    public void setRemoteDebugging(boolean value) {
        mRemoteDebugging = value;
    }

    public void setUseAnimatableView(boolean value) {
        mUseAnimatableView = value;
    }

    @Override
    public boolean isXWalkReady() {
        return super.isXWalkReady();
    }
}
