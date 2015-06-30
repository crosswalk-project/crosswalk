// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.util.Log;
import android.view.Window;

import org.xwalk.core.XWalkLibraryLoader.ActivateListener;
import org.xwalk.core.XWalkUpdater.XWalkUpdateListener;

public class XWalkActivityDelegate implements ActivateListener, XWalkUpdateListener {
    private static final String TAG = "XWalkActivity";

    private Activity mActivity;
    private XWalkDialogManager mDialogManager;
    private XWalkUpdater mXWalkUpdater;
    private Runnable mCancelCommand;
    private Runnable mCompleteCommand;

    private boolean mIsInitializing;
    private boolean mIsXWalkReady;
    private boolean mBackgroundDecorated;

    public XWalkActivityDelegate(Activity activity,
            Runnable cancelCommand, Runnable completeCommand) {
        mActivity = activity;
        mCancelCommand = cancelCommand;
        mCompleteCommand = completeCommand;

        mDialogManager = new XWalkDialogManager(mActivity);
        mXWalkUpdater = new XWalkUpdater(this, mActivity, mDialogManager);

        XWalkLibraryLoader.prepareToInit(mActivity);
    }

    public boolean isXWalkReady() {
        return mIsXWalkReady;
    }

    public boolean isSharedMode() {
        return mIsXWalkReady && XWalkLibraryLoader.isSharedLibrary();
    }

    public void setXWalkApkUrl(String url) {
        mXWalkUpdater.setXWalkApkUrl(url);
    }

    public void onResume() {
        if (mIsInitializing || mIsXWalkReady) return;

        mIsInitializing = true;
        Log.d(TAG, "Activate by XWalkActivity");
        XWalkLibraryLoader.startActivate(this, mActivity);
    }

    @Override
    public void onActivateStarted() {
    }

    @Override
    public void onActivateFailed() {
        mIsInitializing = false;

        if (mXWalkUpdater.updateXWalkRuntime()) {
            // Set the background to screen_background_dark temporarily if the default background
            // is null in order to avoid the visual artifacts around the alert dialog
            Window window = mActivity.getWindow();
            if (window != null && window.getDecorView().getBackground() == null) {
                Log.d(TAG, "Set the background to screen_background_dark");
                window.setBackgroundDrawableResource(android.R.drawable.screen_background_dark);
                mBackgroundDecorated = true;
            }
        }
    }

    @Override
    public void onActivateCompleted() {
        if (mDialogManager.isShowingDialog()) mDialogManager.dismissDialog();

        if (mBackgroundDecorated) {
            Log.d(TAG, "Recover the background");
            mActivity.getWindow().setBackgroundDrawable(null);
            mBackgroundDecorated = false;
        }

        mIsInitializing = false;
        mIsXWalkReady = true;
        mCompleteCommand.run();
    }

    @Override
    public void onXWalkUpdateCancelled() {
        mCancelCommand.run();
    }
}
