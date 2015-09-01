// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DownloadManager;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.util.Log;

import junit.framework.Assert;

class XWalkDialogManager {
    private static final String TAG = "XWalkLib";
    private static final String PACKAGE_RE = "[a-z]+\\.[a-z0-9]+\\.[a-z0-9]+.*";

    private Context mContext;
    private Dialog mActiveDialog;
    private String mApplicationName;

    public XWalkDialogManager(Context context) {
        mContext = context;
    }

    private void showDialog(Dialog dialog) {
        mActiveDialog = dialog;
        mActiveDialog.show();
    }

    public void dismissDialog() {
        mActiveDialog.dismiss();
        mActiveDialog = null;
    }

    public boolean isShowingDialog() {
        return mActiveDialog != null && mActiveDialog.isShowing();
    }

    public boolean isShowingProgressDialog() {
        return isShowingDialog() && mActiveDialog instanceof ProgressDialog;
    }

    public void setProgress(int progress, int max) {
        ProgressDialog dialog = (ProgressDialog) mActiveDialog;
        dialog.setIndeterminate(false);
        dialog.setMax(max);
        dialog.setProgress(progress);
    }

    public void showInitializationError(int status, Runnable cancelCommand,
            Runnable downloadCommand) {
        AlertDialog dialog = buildAlertDialog();
        String cancelText = mContext.getString(R.string.xwalk_close);
        String downloadText = mContext.getString(R.string.xwalk_get_crosswalk);

        if (status == XWalkLibraryInterface.STATUS_NOT_FOUND) {
            dialog.setTitle(mContext.getString(R.string.startup_not_found_title));
            dialog.setMessage(replaceApplicationName(mContext.getString(
                    R.string.startup_not_found_message)));
            setPositiveButton(dialog, downloadText, downloadCommand);
            setNegativeButton(dialog, cancelText, cancelCommand);
        } else if (status == XWalkLibraryInterface.STATUS_ARCHITECTURE_MISMATCH) {
            dialog.setTitle(mContext.getString(R.string.startup_architecture_mismatch_title));
            dialog.setMessage(replaceApplicationName(mContext.getString(
                    R.string.startup_architecture_mismatch_message)));
            setPositiveButton(dialog, downloadText, downloadCommand);
            setNegativeButton(dialog, cancelText, cancelCommand);
        } else if (status == XWalkLibraryInterface.STATUS_SIGNATURE_CHECK_ERROR) {
            dialog.setTitle(mContext.getString(R.string.startup_signature_check_error_title));
            dialog.setMessage(replaceApplicationName(mContext.getString(
                    R.string.startup_signature_check_error_message)));
            setNegativeButton(dialog, cancelText, cancelCommand);
        } else if (status == XWalkLibraryInterface.STATUS_OLDER_VERSION) {
            dialog.setTitle(mContext.getString(R.string.startup_older_version_title));
            dialog.setMessage(replaceApplicationName(mContext.getString(
                    R.string.startup_older_version_message)));
            setPositiveButton(dialog, downloadText, downloadCommand);
            setNegativeButton(dialog, cancelText, cancelCommand);
        } else if (status == XWalkLibraryInterface.STATUS_NEWER_VERSION) {
            dialog.setTitle(mContext.getString(R.string.startup_newer_version_title));
            dialog.setMessage(replaceApplicationName(mContext.getString(
                    R.string.startup_newer_version_message)));
            setNegativeButton(dialog, cancelText, cancelCommand);
        } else {
            Assert.fail();
        }
        showDialog(dialog);
    }

    public void showMarketOpenError(Runnable cancelCommand) {
        AlertDialog dialog = buildAlertDialog();
        dialog.setTitle(mContext.getString(R.string.crosswalk_install_title));
        dialog.setMessage(mContext.getString(R.string.market_open_failed_message));
        setNegativeButton(dialog, mContext.getString(R.string.xwalk_close), cancelCommand);
        showDialog(dialog);
    }

    public void showDecompressProgress(Runnable cancelCommand) {
        ProgressDialog dialog = buildProgressDialog();
        dialog.setTitle(mContext.getString(R.string.crosswalk_install_title));
        dialog.setMessage(mContext.getString(R.string.decompression_progress_message));
        setNegativeButton(dialog, mContext.getString(R.string.xwalk_cancel), cancelCommand);
        showDialog(dialog);
    }

    public void showDownloadProgress(Runnable cancelCommand) {
        ProgressDialog dialog = buildProgressDialog();
        dialog.setTitle(mContext.getString(R.string.crosswalk_install_title));
        dialog.setMessage(mContext.getString(R.string.download_progress_message));
        dialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
        setNegativeButton(dialog, mContext.getString(R.string.xwalk_cancel), cancelCommand);
        showDialog(dialog);

    }

    public void showDownloadError(int status, int error, Runnable cancelCommand,
            Runnable downloadCommand) {
        String message = mContext.getString(R.string.download_failed_message);
        if (status == DownloadManager.STATUS_FAILED) {
            if (error == DownloadManager.ERROR_DEVICE_NOT_FOUND) {
                message = mContext.getString(R.string.download_failed_device_not_found) ;
            } else if (error == DownloadManager.ERROR_INSUFFICIENT_SPACE) {
                message = mContext.getString(R.string.download_failed_insufficient_space);
            }
        } else if (status == DownloadManager.STATUS_PAUSED) {
            message = mContext.getString(R.string.download_failed_time_out);
        }

        AlertDialog dialog = buildAlertDialog();
        dialog.setTitle(mContext.getString(R.string.crosswalk_install_title));
        dialog.setMessage(message);
        setPositiveButton(dialog, mContext.getString(R.string.xwalk_retry), downloadCommand);
        setNegativeButton(dialog, mContext.getString(R.string.xwalk_cancel), cancelCommand);
        showDialog(dialog);
    }

    private ProgressDialog buildProgressDialog() {
        ProgressDialog dialog = new ProgressDialog(mContext);
        dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        dialog.setIndeterminate(true);
        dialog.setCancelable(false);
        dialog.setCanceledOnTouchOutside(false);
        return dialog;
    }

    private AlertDialog buildAlertDialog() {
        AlertDialog dialog = new AlertDialog.Builder(mContext).create();
        dialog.setIcon(android.R.drawable.ic_dialog_alert);
        dialog.setCancelable(false);
        dialog.setCanceledOnTouchOutside(false);
        return dialog;
    }

    private void setPositiveButton(AlertDialog dialog, String text, final Runnable command) {
        dialog.setButton(DialogInterface.BUTTON_POSITIVE, text,
                new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int id) {
                        command.run();
                    }
                });
    }

    private void setNegativeButton(AlertDialog dialog, String text, final Runnable command) {
        dialog.setButton(DialogInterface.BUTTON_NEGATIVE, text,
                new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int id) {
                        command.run();
                    }
                });
    }

    private String replaceApplicationName(String text) {
        if (mApplicationName == null) {
            try {
                PackageManager packageManager = mContext.getPackageManager();
                ApplicationInfo appInfo = packageManager.getApplicationInfo(
                        mContext.getPackageName(), 0);
                mApplicationName = (String) packageManager.getApplicationLabel(appInfo);
            } catch (NameNotFoundException e) {
            }

            if (mApplicationName == null || mApplicationName.matches(PACKAGE_RE)) {
                mApplicationName = "this application";
            }
            Log.d(TAG, "Crosswalk application name: " + mApplicationName);
        }

        text = text.replaceAll("APP_NAME", mApplicationName);
        if (text.startsWith("this")) {
            text = text.replaceFirst("this", "This");
        }
        return text;
    }
}
