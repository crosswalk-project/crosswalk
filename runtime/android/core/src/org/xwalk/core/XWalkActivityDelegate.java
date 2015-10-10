// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.util.Log;
import android.view.Window;

import org.xwalk.core.XWalkLibraryLoader.ActivateListener;
import org.xwalk.core.XWalkLibraryLoader.DecompressListener;

public class XWalkActivityDelegate
            implements DecompressListener, ActivateListener {
    /**
     * Interface used to listen to XWalk background update events
     */
    public interface XWalkUpdateListener {
        /**
         * Run on the UI thread to notify the update is started.
         */
        public void onXWalkUpdateStarted();

        /**
         * Run on the UI thread to notify the update progress.
         * @param percentage The update progress in percentage.
         */
        public void onXWalkUpdateProgress(int percentage);

        /**
         * Run on the UI thread to notify the update is cancelled.
         */
        public void onXWalkUpdateCanceled();

        /**
         * Run on the UI thread to notify the update failed.
         */
        public void onXWalkUpdateFailed();

        /**
         * Run on the UI thread to notify the update is completed.
         */
        public void onXWalkUpdateCompleted();
    }

    private static final String TAG = "XWalkActivity";

    private Activity mActivity;
    private XWalkDialogManager mDialogManager;
    private XWalkUpdater mXWalkUpdater;
    private Runnable mCancelCommand;
    private Runnable mCompleteCommand;
    private XWalkUpdateListener mUpdateListener;

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
        mXWalkUpdater = new XWalkUpdater(
            new XWalkUpdater.XWalkUpdateListener() {
                @Override
                public void onXWalkUpdateCancelled() {
                    mCancelCommand.run();
                }
            },
            mActivity,
            mDialogManager);

        XWalkLibraryLoader.prepareToInit(mActivity);
    }

    public XWalkActivityDelegate(Activity activity,
            XWalkUpdateListener listener) {
        mActivity = activity;
        mUpdateListener = listener;
        Log.d(TAG, "Request to run in silent download mode");

        mXWalkUpdater = new XWalkUpdater(
            new XWalkUpdater.XWalkBackgroundUpdateListener() {
                @Override
                public void onXWalkUpdateStarted() {
                    mUpdateListener.onXWalkUpdateStarted();
                }

                @Override
                public void onXWalkUpdateProgress(int percentage) {
                    mUpdateListener.onXWalkUpdateProgress(percentage);
                }

                @Override
                public void onXWalkUpdateCancelled() {
                    mUpdateListener.onXWalkUpdateCanceled();
                }

                @Override
                public void onXWalkUpdateFailed() {
                    mUpdateListener.onXWalkUpdateFailed();
                }

                @Override
                public void onXWalkUpdateCompleted() {
                    XWalkLibraryLoader.startActivate(XWalkActivityDelegate.this,
                        mActivity);
                }
            },
            mActivity);

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
        if (mUpdateListener == null) {
            mDialogManager.showDecompressProgress(new Runnable() {
                @Override
                public void run() {
                    Log.d(TAG, "Cancel by XWalkActivity");
                    XWalkLibraryLoader.cancelDecompress();
                }
            });
        }
        mWillDecompress = true;
    }

    @Override
    public void onDecompressCancelled() {
        mWillDecompress = false;
        mIsInitializing = false;
        if (mUpdateListener == null) {
            mDialogManager.dismissDialog();
            mCancelCommand.run();
        }
    }

    @Override
    public void onDecompressCompleted() {
        if (mWillDecompress) {
            if (mUpdateListener == null)
                mDialogManager.dismissDialog();
            mWillDecompress = false;
        }

        XWalkLibraryLoader.startActivate(this, mActivity);
    }

    @Override
    public void onActivateStarted() {
    }

    @Override
    public void onActivateFailed() {
        mIsInitializing = false;

        if (mXWalkUpdater.updateXWalkRuntime()) {
            if (mUpdateListener == null) {
                // Set the background to screen_background_dark temporarily if
                // the default background is null in order to avoid the visual
                // artifacts around the alert dialog
                Window window = mActivity.getWindow();
                if (window != null &&
                        window.getDecorView().getBackground() == null) {
                    Log.d(TAG, "Set the background to screen_background_dark");
                    window.setBackgroundDrawableResource(
                            android.R.drawable.screen_background_dark);
                    mBackgroundDecorated = true;
                }
            }
        }
    }

    @Override
    public void onActivateCompleted() {
        mIsInitializing = false;
        mIsXWalkReady = true;
        if (mUpdateListener == null) {
            if (mDialogManager.isShowingDialog())
                mDialogManager.dismissDialog();

            if (mBackgroundDecorated) {
                Log.d(TAG, "Recover the background");
                mActivity.getWindow().setBackgroundDrawable(null);
                mBackgroundDecorated = false;
            }

            mCompleteCommand.run();
        } else {
            mUpdateListener.onXWalkUpdateCompleted();
        }
    }
}
