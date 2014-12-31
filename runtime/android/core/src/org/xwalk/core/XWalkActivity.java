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
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;

import java.io.File;
import java.lang.Thread;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.LinkedList;
import java.util.Iterator;

import org.xwalk.core.XWalkCoreWrapper.XWalkCoreStatus;

/**
 * XWalkActiviy is to support cross package resource loading.
 * It provides method to allow overriding getResources() behavior.
 */
public abstract class XWalkActivity extends Activity
        implements XWalkCoreWrapper.XWalkCoreListener {
    private static final String XWALK_CORE_APK = "XWalkRuntimeLib.apk";
    
    private static final String XWALK_CORE_MARKET_URL =
            "market://details?id=" + XWalkCoreWrapper.XWALK_CORE_PACKAGE;

    private static final String XWALK_CORE_DOWNLOAD_URL =
            "http://10.0.2.2/" + XWALK_CORE_APK; // localhost for test

    private DownloadTask mDownloadTask;
    private Dialog mActiveDialog;
    private boolean mIsDisplayed;

    private XWalkCoreWrapper mCoreWrapper;
    private LinkedList<Object> mReservedObjects;
    private LinkedList<ReflectMethod> mReservedMethods;

    public enum XWalkLibraryStatus {
        MATCHED,
        NOT_FOUND,
        NEWER_VERSION,
        OLDER_VERSION,
        COMPRESSED,
    }

    private static XWalkLibraryStatus convertXWalkCoreStatus(XWalkCoreStatus status) {
        return XWalkLibraryStatus.valueOf(status.toString());
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mDownloadTask = null;
        mActiveDialog = null;
        mIsDisplayed = false;
        
        mCoreWrapper = null;
        mReservedObjects = new LinkedList<Object>();
        mReservedMethods = new LinkedList<ReflectMethod>();

        XWalkCoreWrapper.reset(this, mCoreWrapper);
        XWalkCoreWrapper.check();
    }

    @Override
    protected void onResume() {
        super.onResume();

        if (!mIsDisplayed) {
            mIsDisplayed = true;
            return;
        }

        XWalkCoreWrapper.reset(this, mCoreWrapper);
        if (isXWalkReady()) {
            return;
        } else if (mActiveDialog != null && mActiveDialog.isShowing()) {
            // do nothing
        } else {
            XWalkCoreWrapper.check();
        }
    }
    
    @Override
    public void reserveReflectObject(Object object) {
        Log.d("XWalkActivity", "reserve reflect object, " + object.getClass());
        mReservedObjects.add(object);
    }

    @Override
    public void reserveReflectMethod(ReflectMethod method) {
        Log.d("XWalkActivity", "reserve reflect method, " + method);
        mReservedMethods.add(method);
    }

    @Override
    public void onXWalkCoreReady() {
        Log.d("XWalkActivity", "xwalk ready");
        XWalkCoreWrapper.init();
        mCoreWrapper = XWalkCoreWrapper.getInstance();
        
        Object object = mReservedObjects.poll();
        while (object != null) {
            try {
                Log.d("XWalkActivity", "reflectionInit: " + object.getClass());
                Method method = object.getClass().getMethod("reflectionInit");
                method.invoke(object);
            } catch (NoSuchMethodException | IllegalAccessException |
                    IllegalArgumentException | InvocationTargetException e) {
                assert(false);
            }
            object = mReservedObjects.poll();
        }

        ReflectMethod method = mReservedMethods.poll();
        while (method != null) {
            Log.d("XWalkActivity", "recall: " + method);
            method.invokeWithReservedArguments();
            method = mReservedMethods.poll();
        }

        onXWalkReady();
    }

    @Override
    public void onXWalkCoreStartupError(Throwable e, XWalkCoreStatus status) {
        onXWalkStartupError(e, convertXWalkCoreStatus(status));
    }

    @Override
    public void onXWalkCoreRuntimeError(Throwable e, XWalkCoreStatus status) {
        onXWalkRuntimeError(e, convertXWalkCoreStatus(status));
    }
    
    protected abstract void onXWalkReady();

    protected void onXWalkStartupError(Throwable e, XWalkLibraryStatus status) {
        Log.d("XWalkActivity", "xwalk startup error: " + status);
        e.printStackTrace();
        
        Dialog dialog = null;
        if (status == XWalkLibraryStatus.NOT_FOUND) {
            dialog = new StartupNotFoundDialog(this);
        } else if (status == XWalkLibraryStatus.OLDER_VERSION) {
            dialog = new StartupOlderVersionDialog(this);
        } else if (status == XWalkLibraryStatus.NEWER_VERSION) {
            dialog = new StartupNewerVersionDialog(this);
        } else if  (status == XWalkLibraryStatus.COMPRESSED) {
            dialog = new StartupDecompressDialog(this);
        }

        dialog.show();
    }

    protected void onXWalkRuntimeError(Throwable e, XWalkLibraryStatus status) {
        Log.d("XWalkActivity", "xwalk runtime error: " + status);
        e.printStackTrace();
        
        Dialog dialog = null;
        if (status == XWalkLibraryStatus.OLDER_VERSION) {
            dialog = new RuntimeOlderVersionDialog(this);
        } else if (status == XWalkLibraryStatus.NEWER_VERSION) {
            dialog = new RuntimeNewerVersionDialog(this);
        }
        dialog.show();
    }

    @Override
    public Resources getResources() {
        return getApplicationContext().getResources();
    }

    public String getStringResource(String label) {
        int resId = getResources().getIdentifier(label, "string", getPackageName());
        if (resId == 0) return null;
        return getString(resId);
    }

    public boolean isDisplayed() {
        return mIsDisplayed;
    }

    public boolean isXWalkReady() {
        return mCoreWrapper != null;
    }

    public boolean isSharedMode() {
        return mCoreWrapper.isSharedMode();
    }

    private void getXWalkLibrary() {
        try {
            Intent intent = new Intent(Intent.ACTION_VIEW);
            startActivity(intent.setData(Uri.parse(XWALK_CORE_MARKET_URL)));

            new MarketProgressDialog(this).show();
        } catch (ActivityNotFoundException e) {
            Log.d("XWalkActivity", "Failed to open market");
            downloadXWalkLibrary();
        }
    }

    private void downloadXWalkLibrary() {
        mDownloadTask = new DownloadTask(this);
        mDownloadTask.execute();
    }

    private void installXWalkLibrary(Uri uri) {
        Log.d("XWalkActivity", "Install xwalk library, " + uri.toString());
        Intent install = new Intent(Intent.ACTION_VIEW);
        install.setDataAndType(uri, "application/vnd.android.package-archive");
        startActivity(install);
    }

    private static class DownloadTask extends AsyncTask<Void, Integer, Integer> {
        private static final int QUERY_INTERVAL_MS = 100;
        private static final int MAX_PAUSED_COUNT = 50;
        private static final int MAX_RUNNING_COUNT = 300;
        
        XWalkActivity mXWalkActivity;
        DownloadManager mDownloadManager;
        long mDownloadId;
        ProgressDialog mProgressDialog;

        DownloadTask(XWalkActivity activity) {
            super();

            mXWalkActivity = activity;
            mDownloadManager= (DownloadManager) activity.getSystemService(DOWNLOAD_SERVICE);
            mDownloadId = -1;
            mProgressDialog = new DownloadProgressDialog(activity);
        }

        @Override
        protected void onPreExecute() {
            Log.d("XWalkActivity", "Download started, " + XWalkActivity.XWALK_CORE_DOWNLOAD_URL);
            Request request = new Request(Uri.parse(XWalkActivity.XWALK_CORE_DOWNLOAD_URL));
            request.setDestinationInExternalPublicDir(
                    Environment.DIRECTORY_DOWNLOADS, XWALK_CORE_APK);
            mDownloadId = mDownloadManager.enqueue(request);

            mProgressDialog.setIndeterminate(true);
            mProgressDialog.show();
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
                if (cursor != null && cursor.moveToFirst()) {
                    int totalIdx = cursor.getColumnIndex(DownloadManager.COLUMN_TOTAL_SIZE_BYTES);
                    int downloadIdx = cursor.getColumnIndex(
                            DownloadManager.COLUMN_BYTES_DOWNLOADED_SO_FAR);
                    int totalSize = cursor.getInt(totalIdx);
                    int downloadSize = cursor.getInt(downloadIdx);
                    if (totalSize > 0) {
                        publishProgress(downloadSize, totalSize);
                    }
                    
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
            }
            
            return 0;
        }

        @Override
        protected void onProgressUpdate(Integer... progress) {
            Log.d("XWalkActivity", "Download progress: " + progress[0] + "/" + progress[1]);
            mProgressDialog.setProgress(progress[0]);
            mProgressDialog.setMax(progress[1]);
            mProgressDialog.setIndeterminate(false);
        }

        @Override
        protected void onCancelled(Integer result) {
            Log.d("XWalkActivity", "Download cancelled");
            mXWalkActivity.finish();
        }

        @Override
        protected void onPostExecute(Integer result) {
            mProgressDialog.dismiss();

            if (result == DownloadManager.STATUS_SUCCESSFUL) {
                Log.d("XWalkActivity", "Download finished");
                Uri uri = mDownloadManager.getUriForDownloadedFile(mDownloadId);
                mXWalkActivity.installXWalkLibrary(uri);
            } else {
                String errMsg = XWalkMixedResources.DOWNLOAD_FAILED_MESSAGE;
                if (result == DownloadManager.STATUS_FAILED) {
                    Query query = new Query().setFilterById(mDownloadId);
                    Cursor cursor = mDownloadManager.query(query);
                    if (cursor != null && cursor.moveToFirst()) {
                        int reasonIdx = cursor.getColumnIndex(DownloadManager.COLUMN_REASON);
                        int reason = cursor.getInt(reasonIdx);
                        if (reason == DownloadManager.ERROR_DEVICE_NOT_FOUND) {
                            errMsg = XWalkMixedResources.DOWNLOAD_FAILED_DEVICE_NOT_FOUND ;
                        } else if (reason == DownloadManager.ERROR_INSUFFICIENT_SPACE) {
                            errMsg = XWalkMixedResources.DOWNLOAD_FAILED_INSUFFICIENT_SPACE ;
                        }
                    }
                } else if (result == DownloadManager.STATUS_PAUSED) {
                } else if (result == DownloadManager.STATUS_RUNNING) {
                    errMsg = XWalkMixedResources.DOWNLOAD_FAILED_TIME_OUT;
                } else {
                    assert(false);
                }

                Log.d("XWalkActivity", "Download failed, " + errMsg);
                new DownloadFailedDialog(mXWalkActivity, errMsg).show();
            }
        }
    }
    
    private static class DecompressTask extends AsyncTask<Void, Integer, Void> {
        XWalkActivity mXWalkActivity;
        AlertDialog mDialog;

        DecompressTask(XWalkActivity activity, AlertDialog dialog) {
            super();
            mXWalkActivity = activity;
            mDialog = dialog;
        }

        @Override
        protected Void doInBackground(Void... params) {
            try {
                XWalkCoreWrapper.decompressXWalkLibrary();
                // TODO: use publishProgress to update percentage.
            } catch (Exception e) {
                Log.w("XWalkActivity", "Decompress library failed: " + e.getMessage());
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... progress) {
            // TODO: update percentage.
        }

        @Override
        protected void onPostExecute(Void v) {
            mXWalkActivity.onXWalkCoreReady();
            mDialog.dismiss();
        }

        @Override
        protected void onCancelled(Void v) {
            Log.d("XWalkActivity", "Decompress cancelled");
            mXWalkActivity.finish();
        }
    }

    private static class DownloadProgressDialog extends ProgressDialog {
        XWalkActivity mXWalkActivity;
        
        DownloadProgressDialog(XWalkActivity activity) {
            super(activity);
            mXWalkActivity = activity;

            OnShowListener showListener = new OnShowListener() {
                @Override
                public void onShow(DialogInterface dialog) {
                    mXWalkActivity.mActiveDialog = DownloadProgressDialog.this;
                }
            };
            OnClickListener negativeListener = new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int id) {
                    mXWalkActivity.mDownloadTask.cancel(true);
                }
            };

            setMessage(XWalkMixedResources.DOWNLOAD_PROGRESS_MESSAGE);
            setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
            
            setOnShowListener(showListener);
            setButton(DialogInterface.BUTTON_NEGATIVE,
                    XWalkMixedResources.CANCEL, negativeListener);
            setCancelable(false);
        }
    }

    private static class DownloadFailedDialog extends AlertDialog {
        XWalkActivity mXWalkActivity;
        
        DownloadFailedDialog(XWalkActivity activity, String errMsg) {
            super(activity);
            mXWalkActivity = activity;

            OnShowListener showListener = new OnShowListener() {
                @Override
                public void onShow(DialogInterface dialog) {
                    mXWalkActivity.mActiveDialog = DownloadFailedDialog.this;
                }
            };
            OnClickListener positiveListener = new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int id) {
                    mXWalkActivity.downloadXWalkLibrary();
                }
            };
            OnClickListener negativeListener = new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int id) {
                    mXWalkActivity.finish();
                }
            };

            setIcon(android.R.drawable.ic_dialog_alert);
            setTitle(XWalkMixedResources.DOWNLOAD_FAILED_TITLE);
            setMessage(errMsg);
            
            setOnShowListener(showListener);
            setButton(DialogInterface.BUTTON_POSITIVE,
                    XWalkMixedResources.RETRY, positiveListener);
            setButton(DialogInterface.BUTTON_NEGATIVE,
                    XWalkMixedResources.CANCEL, negativeListener);
            setCancelable(false);
        }
    }

    private static class MarketProgressDialog extends ProgressDialog {
        XWalkActivity mXWalkActivity;
        BroadcastReceiver mInstallReceiver;

        MarketProgressDialog(XWalkActivity activity) {
            super(activity);
            mXWalkActivity = activity;

            mInstallReceiver = new BroadcastReceiver() {
                @Override
                public void onReceive(Context context, Intent intent) {
                    Uri uri = intent.getData();
                    if (!uri.getEncodedSchemeSpecificPart().equals(
                            XWalkCoreWrapper.XWALK_CORE_PACKAGE)) {
                        return;
                    }

                    Log.d("XWalkActivity", "Installed xwalk library from market");
                    if (isShowing()) XWalkCoreWrapper.check();
                    dismiss();
                }
            };

            OnShowListener showListener = new OnShowListener() {
                @Override
                public void onShow(DialogInterface dialog) {
                    mXWalkActivity.mActiveDialog = MarketProgressDialog.this;
                    
                    IntentFilter filter = new IntentFilter(Intent.ACTION_PACKAGE_ADDED);
                    filter.addAction(Intent.ACTION_PACKAGE_CHANGED);
                    filter.addDataScheme("package");
                    mXWalkActivity.registerReceiver(mInstallReceiver, filter);
                }
            };
            OnDismissListener dismissListener = new OnDismissListener() {
                @Override
                public void onDismiss(DialogInterface dialog) {
                    mXWalkActivity.unregisterReceiver(mInstallReceiver);
                }
            };
            OnClickListener negativeListener = new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int id) {
                    mXWalkActivity.finish();
                }
            };

            setMessage(XWalkMixedResources.MARKET_PROGRESS_MESSAGE);
            setIndeterminate(true);
            setProgressStyle(ProgressDialog.STYLE_SPINNER);
            
            setOnShowListener(showListener);
            setOnDismissListener(dismissListener);
            setButton(DialogInterface.BUTTON_NEGATIVE,
                    XWalkMixedResources.CANCEL, negativeListener);
            setCancelable(false);
        }
    }

    private static class StartupNotFoundDialog extends AlertDialog {
        XWalkActivity mXWalkActivity;

        StartupNotFoundDialog(XWalkActivity activity) {
            super(activity);
            mXWalkActivity = activity;

            OnShowListener showListener = new OnShowListener() {
                @Override
                public void onShow(DialogInterface dialog) {
                    mXWalkActivity.mActiveDialog = StartupNotFoundDialog.this;
                }
            };
            OnClickListener positiveListener = new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int id) {
                    mXWalkActivity.getXWalkLibrary();
                }
            };
            OnClickListener negativeListener = new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int id) {
                    mXWalkActivity.finish();
                }
            };

            setIcon(android.R.drawable.ic_dialog_alert);
            setTitle(XWalkMixedResources.STARTUP_NOT_FOUND_TITLE);
            setMessage(XWalkMixedResources.STARTUP_NOT_FOUND_MESSAGE);
            
            setOnShowListener(showListener);
            setButton(DialogInterface.BUTTON_POSITIVE,
                    XWalkMixedResources.GET_LIBRARY, positiveListener);
            setButton(DialogInterface.BUTTON_NEGATIVE,
                    XWalkMixedResources.CANCEL, negativeListener);
            setCancelable(false);
        }
    }

    private static class StartupOlderVersionDialog extends AlertDialog {
        XWalkActivity mXWalkActivity;

        StartupOlderVersionDialog(XWalkActivity activity) {
            super(activity);
            mXWalkActivity = activity;

            OnShowListener showListener = new OnShowListener() {
                @Override
                public void onShow(DialogInterface dialog) {
                    mXWalkActivity.mActiveDialog = StartupOlderVersionDialog.this;
                }
            };
            OnClickListener positiveListener = new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int id) {
                    mXWalkActivity.getXWalkLibrary();
                }
            };
            OnClickListener negativeListener = new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int id) {
                    mXWalkActivity.onXWalkCoreReady();
                }
            };

            setIcon(android.R.drawable.ic_dialog_alert);
            setTitle(XWalkMixedResources.STARTUP_OLDER_VERSION_TITLE);
            setMessage(XWalkMixedResources.STARTUP_OLDER_VERSION_MESSAGE);
            
            setOnShowListener(showListener);
            setButton(DialogInterface.BUTTON_POSITIVE,
                    XWalkMixedResources.GET_LIBRARY, positiveListener);
            setButton(DialogInterface.BUTTON_NEGATIVE,
                    XWalkMixedResources.CONTINUE, negativeListener);
            setCancelable(false);
        }
    }

    private static class StartupNewerVersionDialog extends AlertDialog {
        XWalkActivity mXWalkActivity;

        StartupNewerVersionDialog(XWalkActivity activity) {
            super(activity);
            mXWalkActivity = activity;

            OnShowListener showListener = new OnShowListener() {
                @Override
                public void onShow(DialogInterface dialog) {
                    mXWalkActivity.mActiveDialog = StartupNewerVersionDialog.this;
                }
            };
            OnClickListener positiveListener = new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int id) {
                    mXWalkActivity.finish();
                }
            };
            OnClickListener negativeListener = new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int id) {
                    mXWalkActivity.onXWalkCoreReady();
                }
            };

            setIcon(android.R.drawable.ic_dialog_alert);
            setTitle(XWalkMixedResources.STARTUP_NEWER_VERSION_TITLE);
            setMessage(XWalkMixedResources.STARTUP_NEWER_VERSION_MESSAGE);
           
            setOnShowListener(showListener);
            setButton(DialogInterface.BUTTON_POSITIVE,
                    XWalkMixedResources.TERMINATE, positiveListener);
            setButton(DialogInterface.BUTTON_NEGATIVE,
                    XWalkMixedResources.CONTINUE, negativeListener);
            setCancelable(false);
        }
    }

    private static class RuntimeOlderVersionDialog extends AlertDialog {
        XWalkActivity mXWalkActivity;

        RuntimeOlderVersionDialog(XWalkActivity activity) {
            super(activity);
            mXWalkActivity = activity;

            OnClickListener positiveListener = new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int id) {
                    mXWalkActivity.finish();
                }
            };
            OnClickListener negativeListener = new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int id) {
                    // do nothing
                }
            };

            setIcon(android.R.drawable.ic_dialog_alert);
            setTitle(XWalkMixedResources.RUNTIME_OLDER_VERSION_TITLE);
            setMessage(XWalkMixedResources.RUNTIME_OLDER_VERSION_MESSAGE);
            
            setButton(DialogInterface.BUTTON_POSITIVE,
                    XWalkMixedResources.TERMINATE, positiveListener);
            setButton(DialogInterface.BUTTON_NEGATIVE,
                    XWalkMixedResources.CONTINUE, negativeListener);
            setCancelable(false);
        }
    }

    private static class RuntimeNewerVersionDialog extends AlertDialog {
        XWalkActivity mXWalkActivity;

        RuntimeNewerVersionDialog(XWalkActivity activity) {
            super(activity);
            mXWalkActivity = activity;

            OnClickListener positiveListener = new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int id) {
                    mXWalkActivity.finish();
                }
            };
            OnClickListener negativeListener = new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int id) {
                    // do nothing
                }
            };

            setIcon(android.R.drawable.ic_dialog_alert);
            setTitle(XWalkMixedResources.RUNTIME_NEWER_VERSION_TITLE);
            setMessage(XWalkMixedResources.RUNTIME_NEWER_VERSION_MESSAGE);
            
            setButton(DialogInterface.BUTTON_POSITIVE,
                    XWalkMixedResources.TERMINATE, positiveListener);
            setButton(DialogInterface.BUTTON_NEGATIVE,
                    XWalkMixedResources.CONTINUE, negativeListener);
            setCancelable(false);
        }
    }

    private static class StartupDecompressDialog extends AlertDialog {
        XWalkActivity mXWalkActivity;

        StartupDecompressDialog(XWalkActivity activity) {
            super(activity);
            mXWalkActivity = activity;

            final DecompressTask decompressTask = new DecompressTask(mXWalkActivity, this);

            OnClickListener positiveListener = new OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int id) {
                    decompressTask.cancel(true);
                }
            };

            setIcon(android.R.drawable.ic_dialog_alert);
            setMessage(XWalkMixedResources.DECOMPRESS_LIBRARY_MESSAGE);
            setButton(DialogInterface.BUTTON_POSITIVE,
                    XWalkMixedResources.TERMINATE, positiveListener);
            setCancelable(false);
            setCanceledOnTouchOutside(false);
            decompressTask.execute();
        }
    }
}
