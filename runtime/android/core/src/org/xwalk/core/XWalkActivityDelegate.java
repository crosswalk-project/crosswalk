// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.util.Log;
import android.view.Window;

import org.xwalk.core.XWalkLibraryLoader.ActivateListener;
import org.xwalk.core.XWalkLibraryLoader.DecompressListener;
import org.xwalk.core.XWalkUpdater.XWalkBackgroundUpdateListener;
import org.xwalk.core.XWalkUpdater.XWalkUpdateListener;

public class XWalkActivityDelegate implements DecompressListener, ActivateListener {
    private static final String TAG = "XWalkLib";

    private Activity mActivity;
    private XWalkDialogManager mDialogManager;
    private XWalkUpdater mXWalkUpdater;
    private Runnable mCancelCommand;
    private Runnable mCompleteCommand;

    private boolean mIsXWalkReady;
    private boolean mBackgroundDecorated;
    private boolean mWillDecompress;

    public XWalkActivityDelegate(Activity activity,
            Runnable cancelCommand, Runnable completeCommand) {
        mActivity = activity;
        mCancelCommand = cancelCommand;
        mCompleteCommand = completeCommand;

        mDialogManager = new XWalkDialogManager(mActivity);

        XWalkLibraryLoader.prepareToInit(mActivity);
    }

    public boolean isXWalkReady() {
        return mIsXWalkReady;
    }

    public boolean isSharedMode() {
        return mIsXWalkReady && XWalkLibraryLoader.isSharedLibrary();
    }

    public boolean isDownloadMode() {
        return mIsXWalkReady && XWalkEnvironment.isDownloadMode();
    }

    public void setXWalkApkUrl(String url) {
        XWalkEnvironment.setXWalkApkUrl(url);
    }

    public XWalkDialogManager getDialogManager() {
        return mDialogManager;
    }

    public void onResume() {
        if (mIsXWalkReady) return;

        if (XWalkLibraryLoader.isInitializing() || XWalkLibraryLoader.isDownloading()) {
            Log.d(TAG, "Other initialization or download is proceeding");
            return;
        }

        Log.d(TAG, "Initialize by XWalkActivity");
        XWalkLibraryLoader.startDecompress(this);
    }

    @Override
    public void onDecompressStarted() {
        mDialogManager.showDecompressProgress(new Runnable() {
            @Override
            public void run() {
                Log.d(TAG, "Cancel by XWalkActivity");
                XWalkLibraryLoader.cancelDecompress();
            }
        });
        mWillDecompress = true;
    }

    @Override
    public void onDecompressCancelled() {
        mWillDecompress = false;
        mCancelCommand.run();
    }

    @Override
    public void onDecompressCompleted() {
        if (mWillDecompress) {
            mDialogManager.dismissDialog();
            mWillDecompress = false;
        }

        XWalkLibraryLoader.startActivate(this);
    }

    @Override
    public void onActivateStarted() {
    }

    @Override
    public void onActivateFailed() {
        if (mXWalkUpdater == null) {
            if (XWalkEnvironment.isDownloadMode()) {
                mXWalkUpdater = new XWalkUpdater(
                    new XWalkBackgroundUpdateListener() {
                        @Override
                        public void onXWalkUpdateStarted() {
                        }

                        @Override
                        public void onXWalkUpdateProgress(int percentage) {
                        }

                        @Override
                        public void onXWalkUpdateCancelled() {
                            mCancelCommand.run();
                        }

                        @Override
                        public void onXWalkUpdateFailed() {
                            mCancelCommand.run();
                        }

                        @Override
                        public void onXWalkUpdateCompleted() {
                            XWalkLibraryLoader.startActivate(XWalkActivityDelegate.this);
                        }
                    },
                    mActivity);
            } else {
                mXWalkUpdater = new XWalkUpdater(
                    new XWalkUpdateListener() {
                        @Override
                        public void onXWalkUpdateCancelled() {
                            mCancelCommand.run();
                        }
                    },
                    mActivity, mDialogManager);
            }
        }

        if (mXWalkUpdater.updateXWalkRuntime() && !XWalkEnvironment.isDownloadMode()) {
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
        if (mDialogManager.isShowingDialog()) {
            mDialogManager.dismissDialog();
        }

        if (mBackgroundDecorated) {
            Log.d(TAG, "Recover the background");
            mActivity.getWindow().setBackgroundDrawable(null);
            mBackgroundDecorated = false;
        }

        mIsXWalkReady = true;
        XWalkLibraryLoader.finishInit(mActivity);
        mCompleteCommand.run();
    }
}
