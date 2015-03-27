// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DownloadManager;
import android.app.DownloadManager.Request;
import android.app.DownloadManager.Query;
import android.app.ProgressDialog;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.DialogInterface.OnDismissListener;
import android.content.DialogInterface.OnShowListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;

import java.lang.Thread;
import java.util.LinkedList;

import junit.framework.Assert;

import org.xwalk.core.XWalkLibraryListener.LibraryStatus;

/**
 * XWalkActiviy is to support cross package resource loading.
 * It provides method to allow overriding getResources() behavior.
 */
public abstract class XWalkActivity extends Activity implements XWalkLibraryListener {
    private static final String XWALK_CORE_APK = "XWalkRuntimeLib.apk";

    private static final String XWALK_CORE_MARKET_URL =
            "market://details?id=" + XWalkCoreWrapper.XWALK_APK_PACKAGE;

    private static final String XWALK_CORE_DOWNLOAD_URL =
            "http://10.0.2.2/" + XWALK_CORE_APK; // This address is only for test temporarily

    private static final String TAG = "XWalkActivity";

    private DownloadTask mDownloadTask;
    private Dialog mActiveDialog;
    private XWalkCoreWrapper mCoreWrapper;

    private boolean mIsXWalkReady;
    private boolean mIsVisible;
    private LinkedList<Object> mReservedObjects;
    private LinkedList<ReflectMethod> mReservedMethods;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mDownloadTask = null;
        mActiveDialog = null;
        mCoreWrapper = null;

        mIsXWalkReady = false;
        mIsVisible = false;
        mReservedObjects = new LinkedList<Object>();
        mReservedMethods = new LinkedList<ReflectMethod>();

        XWalkCoreWrapper.reset(null, this);
    }

    @Override
    protected void onStart() {
        super.onStart();
        mIsVisible = true;
        XWalkCoreWrapper.reset(mCoreWrapper, this);
        if (!isXWalkReady()) XWalkCoreWrapper.check();
    }

    @Override
    protected void onStop() {
        super.onStop();
        mIsVisible = false;
    }

    @Override
    public Resources getResources() {
        return getApplicationContext().getResources();
    }

    @Override
    public final void onObjectInitFailed(Object object) {
        Log.d(TAG, "Reserve object: " + object.getClass());
        mReservedObjects.add(object);
    }

    @Override
    public final void onMethodCallMissed(ReflectMethod method) {
        Log.d(TAG, "Reserve method: " + method.toString());
        mReservedMethods.add(method);
    }

    @Override
    public final void onXWalkLibraryStartupError(LibraryStatus status, Throwable e) {
        onXWalkStartupError(status, e);
    }

    @Override
    public final void onXWalkLibraryRuntimeError(LibraryStatus status, Throwable e) {
        onXWalkRuntimeError(status, e);
    }

    @Override
    public final void onXWalkLibraryCancelled() {
        finish();
    }

    @Override
    public final void onXWalkLibraryMatched() {
        Log.d(TAG, "XWalk library matched");
        XWalkCoreWrapper.init();
        mCoreWrapper = XWalkCoreWrapper.getInstance();

        for (Object object = mReservedObjects.poll(); object != null;
                object = mReservedObjects.poll()) {
            Log.d(TAG, "Init reserved objects: " + object.getClass());
            try {
                new ReflectMethod(null, object, "reflectionInit").invoke();
            } catch (RuntimeException e) {
                Assert.fail("Reflect object initialization failed");
            }
        }

        for (ReflectMethod method = mReservedMethods.poll(); method != null;
                method = mReservedMethods.poll()) {
            Log.d(TAG, "Call reserved methods: " + method);
            Object[] args = method.getArguments();
            if (args != null) {
                for (int i = 0; i < args.length; ++i) {
                    if (args[i] instanceof ReflectMethod) {
                        args[i] = ((ReflectMethod) args[i]).invokeWithArguments();
                    }
                }
            }
            method.invokeWithArguments();
        }

        mIsXWalkReady = true;
        onXWalkReady();
    }

    protected abstract void onXWalkReady();

    protected void onXWalkStartupError(LibraryStatus status, Throwable e) {
        Log.d(TAG, "XWalk startup error: " + status);
        e.printStackTrace();

        AlertDialog dialog = null;
        if (status == LibraryStatus.NOT_FOUND) {
            dialog = getStartupNotFoundDialog();
        } else if (status == LibraryStatus.OLDER_VERSION) {
            dialog = getStartupOlderVersionDialog();
        } else if (status == LibraryStatus.NEWER_VERSION) {
            dialog = getStartupNewerVersionDialog();
        }
        showDialog(dialog);
    }

    protected void onXWalkRuntimeError(LibraryStatus status, Throwable e) {
        Log.d(TAG, "XWalk runtime error: " + status);
        e.printStackTrace();
        Assert.fail();
    }

    protected boolean isXWalkReady() {
        return mIsXWalkReady;
    }

    protected boolean isSharedMode() {
        return mCoreWrapper != null && mCoreWrapper.isSharedMode();
    }

    protected int getSdkVersion() {
        return XWalkSdkVersion.SDK_VERSION;
    }

    protected ProgressDialog buildProgressDialog() {
        return new ProgressDialog(this);
    }

    protected AlertDialog buildAlertDialog() {
        return new AlertDialog.Builder(this).create();
    }

    protected void showDialog(Dialog dialog) {
        mActiveDialog = dialog;
        mActiveDialog.show();
    }

    protected void dismissDialog() {
        mActiveDialog.dismiss();
    }

    private void getXWalkLibrary() {
        try {
            Intent intent = new Intent(Intent.ACTION_VIEW);
            startActivity(intent.setData(Uri.parse(XWALK_CORE_MARKET_URL)));
            showDialog(getMarketProgressDialog());
        } catch (ActivityNotFoundException e) {
            Log.d(TAG, "Market open failed");
            showDialog(getMarketOpenFailedDialog());
            // TODO(sunlin): enable direct download in the future
            // downloadXWalkLibrary();
        }
    }

    private void downloadXWalkLibrary() {
        showDialog(getDownloadProgressDialog());

        mDownloadTask = new DownloadTask(this);
        mDownloadTask.execute();
    }

    private void installXWalkLibrary(Uri uri) {
        Log.d(TAG, "Install xwalk library, " + uri.toString());
        Intent install = new Intent(Intent.ACTION_VIEW);
        install.setDataAndType(uri, "application/vnd.android.package-archive");
        startActivity(install);
    }

    private static class DownloadTask extends AsyncTask<Void, Integer, Integer> {
        private static final int QUERY_INTERVAL_MS = 100;
        private static final int MAX_PAUSED_COUNT = 6000; // 10 minutes
        private static final int MAX_RUNNING_COUNT = 18000; // 30 minutes

        private XWalkActivity mXWalkActivity;
        private DownloadManager mDownloadManager;
        private long mDownloadId;
        private ProgressDialog mProgressDialog;

        DownloadTask(XWalkActivity activity) {
            super();

            mXWalkActivity = activity;
            mDownloadManager= (DownloadManager) activity.getSystemService(DOWNLOAD_SERVICE);
            mDownloadId = -1;
            mProgressDialog = (ProgressDialog) activity.mActiveDialog;
        }

        @Override
        protected void onPreExecute() {
            Log.d(TAG, "Download started, " + XWalkActivity.XWALK_CORE_DOWNLOAD_URL);
            Request request = new Request(Uri.parse(XWalkActivity.XWALK_CORE_DOWNLOAD_URL));
            request.setDestinationInExternalPublicDir(
                    Environment.DIRECTORY_DOWNLOADS, XWALK_CORE_APK);
            mDownloadId = mDownloadManager.enqueue(request);
        }

        @Override
        protected Integer doInBackground(Void... params) {
            Query query = new Query().setFilterById(mDownloadId);
            int pausedCount = 0;
            int runningCount = 0;

            while (!isCancelled()) {
                try {
                    Thread.sleep(QUERY_INTERVAL_MS);
                } catch (InterruptedException e) {
                    break;
                }

                Cursor cursor = mDownloadManager.query(query);
                if (cursor == null || !cursor.moveToFirst()) continue;

                int totalIdx = cursor.getColumnIndex(DownloadManager.COLUMN_TOTAL_SIZE_BYTES);
                int downloadIdx = cursor.getColumnIndex(
                        DownloadManager.COLUMN_BYTES_DOWNLOADED_SO_FAR);
                int totalSize = cursor.getInt(totalIdx);
                int downloadSize = cursor.getInt(downloadIdx);
                if (totalSize > 0) publishProgress(downloadSize, totalSize);

                int statusIdx = cursor.getColumnIndex(DownloadManager.COLUMN_STATUS);
                int status = cursor.getInt(statusIdx);
                if (status == DownloadManager.STATUS_FAILED ||
                        status == DownloadManager.STATUS_SUCCESSFUL) {
                    return status;
                } else if (status == DownloadManager.STATUS_PAUSED) {
                    if (++pausedCount == MAX_PAUSED_COUNT) return status;
                } else if (status == DownloadManager.STATUS_RUNNING) {
                    if (++runningCount == MAX_RUNNING_COUNT) return status;
                }
            }

            return 0;
        }

        @Override
        protected void onProgressUpdate(Integer... progress) {
            Log.d(TAG, "Download progress: " + progress[0] + "/" + progress[1]);
            mProgressDialog.setProgress(progress[0]);
            mProgressDialog.setMax(progress[1]);
            mProgressDialog.setIndeterminate(false);
        }

        @Override
        protected void onCancelled(Integer result) {
            Log.d(TAG, "Download cancelled");
            mProgressDialog.dismiss();
            mXWalkActivity.onXWalkLibraryCancelled();
        }

        @Override
        protected void onPostExecute(Integer result) {
            mProgressDialog.dismiss();

            if (result == DownloadManager.STATUS_SUCCESSFUL) {
                Log.d(TAG, "Download finished");
                Uri uri = mDownloadManager.getUriForDownloadedFile(mDownloadId);
                mXWalkActivity.installXWalkLibrary(uri);
                return;
            }

            String errMsg = mXWalkActivity.getString(R.string.download_failed_message);
            if (result == DownloadManager.STATUS_FAILED) {
                Query query = new Query().setFilterById(mDownloadId);
                Cursor cursor = mDownloadManager.query(query);
                if (cursor != null && cursor.moveToFirst()) {
                    int reasonIdx = cursor.getColumnIndex(DownloadManager.COLUMN_REASON);
                    int reason = cursor.getInt(reasonIdx);
                    if (reason == DownloadManager.ERROR_DEVICE_NOT_FOUND) {
                        errMsg = mXWalkActivity.getString(
                                R.string.download_failed_device_not_found) ;
                    } else if (reason == DownloadManager.ERROR_INSUFFICIENT_SPACE) {
                        errMsg = mXWalkActivity.getString(
                                R.string.download_failed_insufficient_space);
                    }
                }
            } else if (result == DownloadManager.STATUS_PAUSED) {
            } else if (result == DownloadManager.STATUS_RUNNING) {
                errMsg = mXWalkActivity.getString(R.string.download_failed_time_out);
            }

            Log.d(TAG, "Download failed, " + errMsg);
            AlertDialog dialog = mXWalkActivity.getDownloadFailedDialog();
            dialog.setMessage(errMsg);
            mXWalkActivity.showDialog(dialog);
        }
    }

    private ProgressDialog getDownloadProgressDialog() {
        ProgressDialog dialog = buildProgressDialog();

        OnClickListener negativeListener = new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int id) {
                mDownloadTask.cancel(true);
            }
        };

        dialog.setMessage(getString(R.string.download_progress_message));
        dialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
        dialog.setIndeterminate(true);
        dialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                getString(R.string.xwalk_cancel), negativeListener);
        dialog.setCancelable(false);
        return dialog;
    }

    private AlertDialog getDownloadFailedDialog() {
        AlertDialog dialog = buildAlertDialog();

        OnClickListener positiveListener = new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int id) {
                downloadXWalkLibrary();
            }
        };
        OnClickListener negativeListener = new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int id) {
                onXWalkLibraryCancelled();
            }
        };

        dialog.setIcon(android.R.drawable.ic_dialog_alert);
        dialog.setTitle(getString(R.string.download_failed_title));
        dialog.setButton(DialogInterface.BUTTON_POSITIVE,
                getString(R.string.xwalk_retry), positiveListener);
        dialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                getString(R.string.xwalk_cancel), negativeListener);
        dialog.setCancelable(false);
        return dialog;
    }

    private ProgressDialog getMarketProgressDialog() {
        ProgressDialog dialog = buildProgressDialog();

        final BroadcastReceiver installReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                Uri uri = intent.getData();
                if (!uri.getEncodedSchemeSpecificPart().equals(
                        XWalkCoreWrapper.XWALK_APK_PACKAGE)) {
                    return;
                }

                Log.d(TAG, "XWalk library installed");
                dismissDialog();
                if (mIsVisible) XWalkCoreWrapper.check();
            }
        };

        OnShowListener showListener = new OnShowListener() {
            @Override
            public void onShow(DialogInterface dialog) {
                IntentFilter filter = new IntentFilter(Intent.ACTION_PACKAGE_ADDED);
                filter.addAction(Intent.ACTION_PACKAGE_CHANGED);
                filter.addDataScheme("package");
                registerReceiver(installReceiver, filter);
            }
        };
        OnDismissListener dismissListener = new OnDismissListener() {
            @Override
            public void onDismiss(DialogInterface dialog) {
                unregisterReceiver(installReceiver);
            }
        };
        OnClickListener negativeListener = new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int id) {
                onXWalkLibraryCancelled();
            }
        };

        dialog.setMessage(getString(R.string.market_progress_message));
        dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        dialog.setIndeterminate(true);
        dialog.setOnShowListener(showListener);
        dialog.setOnDismissListener(dismissListener);
        dialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                getString(R.string.xwalk_cancel), negativeListener);
        dialog.setCancelable(false);
        return dialog;
    }

    private AlertDialog getStartupNotFoundDialog() {
        AlertDialog dialog = buildAlertDialog();

        OnClickListener positiveListener = new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int id) {
                getXWalkLibrary();
            }
        };

        OnClickListener negativeListener = new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int id) {
                onXWalkLibraryCancelled();
            }
        };

        dialog.setIcon(android.R.drawable.ic_dialog_alert);
        dialog.setTitle(getString(R.string.startup_not_found_title));
        dialog.setMessage(getString(R.string.startup_not_found_message));
        dialog.setButton(DialogInterface.BUTTON_POSITIVE,
                getString(R.string.get_crosswalk), positiveListener);
        dialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                getString(R.string.xwalk_cancel), negativeListener);
        dialog.setCancelable(false);
        return dialog;
    }

    private AlertDialog getStartupOlderVersionDialog() {
        AlertDialog dialog = buildAlertDialog();

        OnClickListener positiveListener = new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int id) {
                getXWalkLibrary();
            }
        };
        OnClickListener negativeListener = new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int id) {
                onXWalkLibraryCancelled();
            }
        };

        dialog.setIcon(android.R.drawable.ic_dialog_alert);
        dialog.setTitle(getString(R.string.startup_older_version_title));
        dialog.setMessage(getString(R.string.startup_older_version_message));
        dialog.setButton(DialogInterface.BUTTON_POSITIVE,
                getString(R.string.get_crosswalk), positiveListener);
        dialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                getString(R.string.xwalk_cancel), negativeListener);
        dialog.setCancelable(false);
        return dialog;
    }

    private AlertDialog getStartupNewerVersionDialog() {
        AlertDialog dialog = buildAlertDialog();

        OnClickListener positiveListener = new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int id) {
                onXWalkLibraryCancelled();
            }
        };

        dialog.setIcon(android.R.drawable.ic_dialog_alert);
        dialog.setTitle(getString(R.string.startup_newer_version_title));
        dialog.setMessage(getString(R.string.startup_newer_version_message));
        dialog.setButton(DialogInterface.BUTTON_POSITIVE,
                getString(R.string.xwalk_cancel), positiveListener);
        dialog.setCancelable(false);
        return dialog;
    }

    private AlertDialog getMarketOpenFailedDialog() {
        AlertDialog dialog = buildAlertDialog();

        OnClickListener positiveListener = new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int id) {
                onXWalkLibraryCancelled();
            }
        };

        dialog.setIcon(android.R.drawable.ic_dialog_alert);
        dialog.setTitle(getString(R.string.market_open_failed_title));
        dialog.setMessage(getString(R.string.market_open_failed_message));
        dialog.setButton(DialogInterface.BUTTON_POSITIVE,
                getString(R.string.xwalk_cancel), positiveListener);
        dialog.setCancelable(false);
        return dialog;
    }
}
