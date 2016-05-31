// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.util.Log;
import android.view.Window;

import org.xwalk.core.XWalkLibraryLoader.ActivateListener;
import org.xwalk.core.XWalkLibraryLoader.DecompressListener;
import org.xwalk.core.XWalkUpdater.XWalkBackgroundUpdateListener;
import org.xwalk.core.XWalkUpdater.XWalkUpdateListener;

public class XWalkActivityDelegate
            implements DecompressListener, ActivateListener {
    private static final String TAG = "XWalkActivity";
    private static final String META_XWALK_ENABLE_DOWNLOAD_MODE = "xwalk_enable_download_mode";
    private static final String META_XWALK_DOWNLOAD_MODE = "xwalk_download_mode";

    private Activity mActivity;
    private XWalkDialogManager mDialogManager;
    private XWalkUpdater mXWalkUpdater;
    private Runnable mCancelCommand;
    private Runnable mCompleteCommand;

    private boolean mIsInitializing;
    private boolean mIsXWalkReady;
    private boolean mBackgroundDecorated;
    private boolean mWillDecompress;
    private final boolean mIsDownloadMode;
    private String mXWalkApkUrl;

    public XWalkActivityDelegate(Activity activity,
            Runnable cancelCommand, Runnable completeCommand) {
        mActivity = activity;
        mCancelCommand = cancelCommand;
        mCompleteCommand = completeCommand;

        String enable = getApplicationMetaData(META_XWALK_DOWNLOAD_MODE);
        if (enable == null) {
            enable = getApplicationMetaData(META_XWALK_ENABLE_DOWNLOAD_MODE);
        }
        mIsDownloadMode = enable != null
                && (enable.equalsIgnoreCase("enable") || enable.equalsIgnoreCase("true"));

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
        return mIsDownloadMode;
    }

    public void setXWalkApkUrl(String url) {
        mXWalkApkUrl = url;
    }

    public XWalkDialogManager getDialogManager() {
        return mDialogManager;
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

        XWalkLibraryLoader.startActivate(this, mActivity);
    }

    @Override
    public void onActivateStarted() {
    }

    @Override
    public void onActivateFailed() {
        mIsInitializing = false;

        if (mXWalkUpdater == null) {
            if (mIsDownloadMode) {
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
                            XWalkLibraryLoader.startActivate(XWalkActivityDelegate.this, mActivity);
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

            if (mXWalkApkUrl != null) {
                mXWalkUpdater.setXWalkApkUrl(mXWalkApkUrl);
            }
        }

        if (mXWalkUpdater.updateXWalkRuntime() && !mIsDownloadMode) {
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
        if (mDialogManager != null && mDialogManager.isShowingDialog()) {
            mDialogManager.dismissDialog();
        }

        if (mBackgroundDecorated) {
            Log.d(TAG, "Recover the background");
            mActivity.getWindow().setBackgroundDrawable(null);
            mBackgroundDecorated = false;
        }

        mIsInitializing = false;
        mIsXWalkReady = true;
        mCompleteCommand.run();
    }

    private String getApplicationMetaData(String name) {
        try {
            PackageManager packageManager = mActivity.getPackageManager();
            ApplicationInfo appInfo = packageManager.getApplicationInfo(
                    mActivity.getPackageName(), PackageManager.GET_META_DATA);
            return appInfo.metaData.getString(name);
        } catch (NameNotFoundException | NullPointerException e) {
        }
        return null;
    }
}
