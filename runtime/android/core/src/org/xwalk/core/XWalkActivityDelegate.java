// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.view.Window;

import org.xwalk.core.XWalkLibraryLoader.ActivateListener;
import org.xwalk.core.XWalkLibraryLoader.DecompressListener;
import org.xwalk.core.XWalkLibraryLoader.DockListener;
import org.xwalk.core.XWalkUpdater.XWalkUpdateListener;

public class XWalkActivityDelegate
            implements DecompressListener, DockListener, ActivateListener, XWalkUpdateListener {
    private Activity mActivity;
    private Runnable mCompleteCommand;
    private Runnable mCancelCommand;
    private XWalkDialogManager mDialogManager;
    private boolean mBackgroundDecorated;

    public XWalkActivityDelegate(Activity activity,
            Runnable completeCommand, Runnable cancelCommand) {
        mActivity = activity;
        mCompleteCommand = completeCommand;
        mCancelCommand = cancelCommand;

        XWalkLibraryLoader.prepareToInit();
    }

    public void initialize() {
        XWalkLibraryLoader.startDecompress(this, mActivity);
    }

    @Override
    public void onDecompressStarted() {
        if (mDialogManager == null) mDialogManager = new XWalkDialogManager(mActivity);
        mDialogManager.showDecompressProgress(new Runnable() {
            @Override
            public void run() {
                XWalkLibraryLoader.cancelDecompress();
            }
        });
    }

    @Override
    public void onDecompressCancelled() {
        mDialogManager.dismissDialog();
        mCancelCommand.run();
    }

    @Override
    public void onDecompressCompleted() {
        mDialogManager.dismissDialog();
        XWalkLibraryLoader.startDock(this, mActivity);
    }

    @Override
    public void onDockStarted() {
    }

    @Override
    public void onDockFailed() {
        if (XWalkUpdater.updateXWalkRuntime(this, mActivity)) {
            // Set background to screen_background_dark temporarily if default background is
            // null to avoid the visual artifacts around the alert dialog
            Window window = mActivity.getWindow();
            if (window != null && window.getDecorView().getBackground() == null) {
                window.setBackgroundDrawableResource(android.R.drawable.screen_background_dark);
                mBackgroundDecorated = true;
            }
        }
    }

    @Override
    public void onDockCompleted() {
        XWalkLibraryLoader.startActivate(this);
    }

    @Override
    public void onActivateStarted() {
    }

    @Override
    public void onActivateCompleted() {
        if (XWalkUpdater.isWaitingForInput()) XWalkUpdater.dismissDialog();

        if (mBackgroundDecorated) {
            mActivity.getWindow().setBackgroundDrawable(null);
            mBackgroundDecorated = false;
        }

        mCompleteCommand.run();
    }

    @Override
    public void onXWalkUpdateCancelled() {
        mCancelCommand.run();
    }
}
