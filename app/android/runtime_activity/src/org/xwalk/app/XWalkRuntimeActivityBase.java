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
import android.widget.Toast;

import org.xwalk.app.runtime.XWalkRuntimeView;
import org.xwalk.core.XWalkActivityDelegate;
import org.xwalk.core.XWalkPreferences;

public abstract class XWalkRuntimeActivityBase extends Activity {
    private static final String TAG = "XWalkRuntimeActivityBase";

    private XWalkRuntimeView mRuntimeView;
    private XWalkActivityDelegate mActivityDelegate;

    private boolean mRemoteDebugging = false;

    private boolean mUseAnimatableView = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Runnable cancelCommand = new Runnable() {
            @Override
            public void run() {
                if (mActivityDelegate.isDownloadMode()) {
                    Toast.makeText(XWalkRuntimeActivityBase.this,
                            "Crosswalk Runtime Update Failed!",
                            Toast.LENGTH_LONG).show();
                }
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

        tryLoadRuntimeView();
    }

    public void onXWalkReady() {
        // XWalkPreferences.ENABLE_EXTENSIONS
        if (XWalkPreferences.getValue("enable-extensions")) {
            // Enable xwalk extension mechanism and start load extensions here.
            // Note that it has to be after above initialization.
            // RuntimeView will finally employ XWalkView to load external extensions.
            mRuntimeView.loadExtensions();
        }
        didTryLoadRuntimeView(mRuntimeView);
        // TODO(sunlin): In shared mode, mRuntimeView.onCreate() will be
        // called after onResume(). Currently, there is no impact because
        // both of onCreate and onResume in XWalkRuntimeView is empty.
        if (mRuntimeView != null) mRuntimeView.onCreate();
    }

    @Override
    public void onStart() {
        super.onStart();
    }

    @Override
    public void onPause() {
        super.onPause();
        if (mRuntimeView != null) mRuntimeView.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
        mActivityDelegate.onResume();
        if (mRuntimeView != null) mRuntimeView.onResume();
    }

    @Override
    public void onStop() {
        super.onStop();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void onNewIntent(Intent intent) {
        if (mRuntimeView == null || !mRuntimeView.onNewIntent(intent)) super.onNewIntent(intent);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (mRuntimeView != null) mRuntimeView.onActivityResult(requestCode, resultCode, data);
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
            setContentView(mRuntimeView);
        } catch (Exception e) {
            handleException(e);
        }
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

    public boolean isXWalkReady() {
        return mActivityDelegate.isXWalkReady();
    }
}
