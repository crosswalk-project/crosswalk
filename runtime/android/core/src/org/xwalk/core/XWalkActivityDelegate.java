// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.util.Log;
import android.view.Window;

import org.xwalk.core.XWalkLibraryLoader.ActivateListener;
import org.xwalk.core.XWalkLibraryLoader.DecompressListener;
import org.xwalk.core.XWalkLibraryLoader.DockListener;
import org.xwalk.core.XWalkUpdater.XWalkUpdateListener;

public class XWalkActivityDelegate
            implements DecompressListener, DockListener, ActivateListener, XWalkUpdateListener {
    private static final String TAG = "XWalkActivity";

    private Activity mActivity;
    private XWalkDialogManager mDialogManager;
    private XWalkUpdater mXWalkUpdater;
    private Runnable mCancelCommand;
    private Runnable mCompleteCommand;
    private Runnable mDecompressCancelCommand;

    private boolean mIsInitializing;
    private boolean mIsXWalkReady;
    private boolean mBackgroundDecorated;
    private boolean mWillDecompress;

    public XWalkActivityDelegate(Activity activity,
            Runnable cancelCommand, Runnable completeCommand) {
        mActivity = activity;
        mCancelCommand = cancelCommand;
        mCompleteCommand = completeCommand;

        mDialogManager = new XWalkDialogManager(mActivity);
        mXWalkUpdater = new XWalkUpdater(this, mActivity, mDialogManager);
        mDecompressCancelCommand = new Runnable() {
            @Override
            public void run() {
                Log.d(TAG, "Cancel by XWalkActivity");
                XWalkLibraryLoader.cancelDecompress();
            }
        };

        XWalkLibraryLoader.prepareToInit(mActivity);
    }

    public boolean isXWalkReady() {
        return mIsXWalkReady;
    }

    public boolean isSharedMode() {
        return mIsXWalkReady && XWalkLibraryLoader.isSharedLibrary();
    }

    public void onResume() {
        if (mIsInitializing || mIsXWalkReady) return;

        mIsInitializing = true;
        if (XWalkLibraryLoader.isLibraryReady()) {
            Log.d(TAG, "Activate by XWalkActivity");
            XWalkLibraryLoader.startActivate(this, mActivity);
        } else {
            Log.d(TAG, "Initialize by XWalkActivity");
            XWalkLibraryLoader.startDecompress(this, mActivity);
        }
    }

    @Override
    public void onDecompressStarted() {
        mDialogManager.showDecompressProgress(mDecompressCancelCommand);
        mWillDecompress = true;
    }

    @Override
    public void onDecompressCancelled() {
        mDialogManager.dismissDialog();
        mWillDecompress = false;

        mIsInitializing = false;
        mCancelCommand.run();
    }

    @Override
    public void onDecompressCompleted() {
        if (mWillDecompress) {
            mDialogManager.dismissDialog();
            mWillDecompress = false;
        }

        XWalkLibraryLoader.startDock(this, mActivity);
    }

    @Override
    public void onDockStarted() {
    }

    @Override
    public void onDockFailed() {
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
    public void onDockCompleted() {
        if (mDialogManager.isShowingDialog()) mDialogManager.dismissDialog();

        if (mBackgroundDecorated) {
            Log.d(TAG, "Recover the background");
            mActivity.getWindow().setBackgroundDrawable(null);
            mBackgroundDecorated = false;
        }

        XWalkLibraryLoader.startActivate(this, mActivity);
    }

    @Override
    public void onActivateStarted() {
    }

    @Override
    public void onActivateCompleted() {
        mIsInitializing = false;
        mIsXWalkReady = true;
        mCompleteCommand.run();
    }

    @Override
    public void onXWalkUpdateCancelled() {
        mCancelCommand.run();
    }
}
