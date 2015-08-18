// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.app.DownloadManager;
import android.app.DownloadManager.Request;
import android.app.DownloadManager.Query;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Environment;
import android.util.Log;

import java.lang.Thread;

import junit.framework.Assert;

/**
 * XWalkLibraryLoader is a low level inteface to schedule decompressing, downloading, activating
 * the Crosswalk runtime. Normal user is recommended to use XWalkActivity or XWalkInitializer which
 * is simpler and more user-friendly.
 *
 * The appropriate invocation order is:
 * prepareToInit() -
 * [ if the Crosswalk runtime is supposed to be compressed - startDecompress() ] -
 * startActivate() -
 * [ if the Crosswalk runtime doesn't match - download suitable version - startActivate() ] - over
 */

class XWalkLibraryLoader {
    /**
     * Interface used to decompress the Crosswalk runtime
     */
    public interface DecompressListener {
        /**
         * Run on the UI thread to notify decompression is started.
         *
         * <p> This method will not be invoked if the Crosswalk runtime is not compressed or has
         * already been decompressed.
         */
        public void onDecompressStarted();

        /**
         * Run on the UI thread to notify decompression is cancelled.
         */
        public void onDecompressCancelled();

        /**
         * Run on the UI thread to notify decompression is completed successfully.
         */
        public void onDecompressCompleted();
    }

    /**
     * Interface used to activate the Crosswalk runtime
     */
    public interface ActivateListener {
        /**
         * Run on the UI thread to notify activation is started
         */
        public void onActivateStarted();

        /**
         * Run on the UI thread to notify activation failed
         */
        public void onActivateFailed();

        /**
         * Run on the UI thread to notify activation is completed successfully
         */
        public void onActivateCompleted();
    }

    /**
     * Interface used to download the Crosswalk runtime
     */
    public interface DownloadListener {
        /**
         * Run on the UI thread to notify download is started
         */
        public void onDownloadStarted();

        /**
         * Run on the UI thread to notify the download progress
         * @param percentage Shows the download progress in percentage
         */
        public void onDownloadUpdated(int percentage);

        /**
         * Run on the UI thread to notify download is cancelled
         */
        public void onDownloadCancelled();

        /**
         * Run on the UI thread to notify download is completed successfully
         * @param uri The Uri where the downloaded file is stored
         */
        public void onDownloadCompleted(Uri uri);

        /**
         * Run on the UI thread to notify download failed
         *
         * @param status The download status defined in android.app.DownloadManager.
         *               The value maybe STATUS_FAILED or STATUS_PAUSED
         * @param error The download error defined in android.app.DownloadManager.
         *              This parameter only makes sense when the status is STATUS_FAILED
         */
        public void onDownloadFailed(int status, int error);
    }

    private static final String XWALK_APK_NAME = "XWalkRuntimeLib.apk";
    private static final String TAG = "XWalkLib";

    private static AsyncTask<Void, Integer, Integer> sActiveTask;

    /**
     * Return true if running in shared mode, false if in embedded mode.
     *
     * <p>This method must be invoked after the Crosswalk runtime has already been initialized
     * successfully.
     */
    public static boolean isSharedLibrary() {
        return XWalkCoreWrapper.getInstance().isSharedMode();
    }

    /**
     * Return true if the Crosswalk runtime has already been initialized successfully either in
     * embedded mode or shared mode, false otherwise.
     */
    public static boolean isLibraryReady() {
        return XWalkCoreWrapper.getInstance() != null;
    }

    /**
     * Return the library status defined in {@link XWalkLibraryInterface}.
     */
    public static int getLibraryStatus() {
        return XWalkCoreWrapper.getCoreStatus();
    }

    /**
     * Prepare to start initializing before all other procedures.
     *
     * <p>This method must be invoked on the UI thread.
     */
    public static void prepareToInit(Activity activity) {
        XWalkCoreWrapper.handlePreInit(activity.getClass().getName());
    }

    /**
     * Start decompressing the Crosswalk runtime in background
     *
     * <p>This method must be invoked on the UI thread.
     *
     * @param listener The {@link DecompressListener} to use
     * @param context The context of the package that holds the compressed Crosswalk runtime
     */
    public static void startDecompress(DecompressListener listener, Context context) {
        new DecompressTask(listener, context).execute();
    }

    /**
     * Attempt to cancel decompression
     *
     * @return False if decompression is not running or could not be cancelled, true otherwise
     */
    public static boolean cancelDecompress() {
        DecompressTask task = (DecompressTask) sActiveTask;
        return task != null && task.cancel(true);
    }

    /**
     * Start activating the Crosswalk runtime in background. The activation is not cancelable.
     *
     * <p>This method must be invoked on the UI thread.
     *
     * @param listener The {@link ActivateListener} to use
     */
    public static void startActivate(ActivateListener listener, Activity activity) {
        new ActivateTask(listener, activity).execute();
    }

    /**
     * Start downloading the Crosswalk runtime in background via Android DownlomadManager
     *
     * <p>This method must be invoked on the UI thread.
     *
     * @param listener The {@link DownloadListener} to use
     * @param context The context to get DownloadManager
     * @param url The URL of the Crosswalk runtime
     */
    public static void startDownload(DownloadListener listener, Context context, String url) {
        new DownloadTask(listener, context, url).execute();
    }

    /**
     * Attempt to cancel download
     *
     * @return False if download is not running or could not be cancelled, true otherwise
     */
    public static boolean cancelDownload() {
        DownloadTask task = (DownloadTask) sActiveTask;
        return task != null && task.cancel(true);
    }

    private static class DecompressTask extends AsyncTask<Void, Integer, Integer> {
        DecompressListener mListener;
        Context mContext;
        boolean mIsCompressed;
        boolean mIsDecompressed;

        DecompressTask(DecompressListener listener, Context context) {
            super();
            mListener = listener;
            mContext = context;
        }

        @Override
        protected void onPreExecute() {
            Log.d(TAG, "DecompressTask started");
            sActiveTask = this;

            mIsCompressed = XWalkLibraryDecompressor.isCompressed(mContext);
            if (mIsCompressed) mIsDecompressed = XWalkLibraryDecompressor.isDecompressed(mContext);
            if (mIsCompressed && !mIsDecompressed) mListener.onDecompressStarted();
        }

        @Override
        protected Integer doInBackground(Void... params) {
            if (!mIsCompressed) return 0;

            if (!mIsDecompressed && !XWalkLibraryDecompressor.decompressLibrary(mContext)) {
                return 1;
            }

            if (!isCancelled() && !XWalkLibraryDecompressor.loadDecompressedLibrary(mContext)) {
                return 2;
            }
            return 0;
        }

        @Override
        protected void onCancelled(Integer result) {
            Log.d(TAG, "DecompressTask cancelled");
            sActiveTask = null;
            mListener.onDecompressCancelled();
        }

        @Override
        protected void onPostExecute(Integer result) {
            Log.d(TAG, "DecompressTask finished, " + result);
            Assert.assertEquals(result.intValue(), 0);
            sActiveTask = null;
            mListener.onDecompressCompleted();
        }
    }

    private static class ActivateTask extends AsyncTask<Void, Integer, Integer> {
        ActivateListener mListener;
        Activity mActivity;

        ActivateTask(ActivateListener listener, Activity activity) {
            super();
            mListener = listener;
            mActivity = activity;
        }

        @Override
        protected void onPreExecute() {
            Log.d(TAG, "ActivateTask started");
            sActiveTask = this;
            mListener.onActivateStarted();
        }

        @Override
        protected Integer doInBackground(Void... params) {
            if (XWalkCoreWrapper.getInstance() != null) return -1;
            return XWalkCoreWrapper.attachXWalkCore(mActivity);
        }

        @Override
        protected void onPostExecute(Integer result) {
            if (result == XWalkLibraryInterface.STATUS_MATCH) {
                XWalkCoreWrapper.dockXWalkCore();
            }
            if (XWalkCoreWrapper.getInstance() != null) {
                XWalkCoreWrapper.handlePostInit(mActivity.getClass().getName());
            }

            Log.d(TAG, "ActivateTask finished, " + result);
            sActiveTask = null;
            if (result > XWalkLibraryInterface.STATUS_MATCH) {
                mListener.onActivateFailed();
            } else {
                mListener.onActivateCompleted();
            }
        }
    }

    private static class DownloadTask extends AsyncTask<Void, Integer, Integer> {
        private static final int QUERY_INTERVAL_MS = 100;
        private static final int MAX_PAUSED_COUNT = 6000; // 10 minutes

        private DownloadListener mListener;
        private String mDownloadUrl;
        private DownloadManager mDownloadManager;
        private long mDownloadId;

        DownloadTask(DownloadListener listener, Context context, String url) {
            super();
            mListener = listener;
            mDownloadUrl = url;
            mDownloadManager = (DownloadManager) context.getSystemService(Context.DOWNLOAD_SERVICE);
        }

        @Override
        protected void onPreExecute() {
            Log.d(TAG, "DownloadTask started, " + mDownloadUrl);
            sActiveTask = this;

            Request request = new Request(Uri.parse(mDownloadUrl));
            request.setDestinationInExternalPublicDir(
                    Environment.DIRECTORY_DOWNLOADS, XWALK_APK_NAME);
            mDownloadId = mDownloadManager.enqueue(request);

            mListener.onDownloadStarted();
        }

        @Override
        protected Integer doInBackground(Void... params) {
            Query query = new Query().setFilterById(mDownloadId);
            int pausedCount = 0;

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
                }
            }

            return DownloadManager.STATUS_RUNNING;
        }

        @Override
        protected void onProgressUpdate(Integer... progress) {
            Log.d(TAG, "DownloadTask updated: " + progress[0] + "/" + progress[1]);
            int percentage = 0;
            if (progress[1] > 0) percentage = (int) (progress[0] * 100.0 / progress[1]);
            mListener.onDownloadUpdated(percentage);
        }

        @Override
        protected void onCancelled(Integer result) {
            mDownloadManager.remove(mDownloadId);

            Log.d(TAG, "DownloadTask cancelled");
            sActiveTask = null;
            mListener.onDownloadCancelled();
        }

        @Override
        protected void onPostExecute(Integer result) {
            Log.d(TAG, "DownloadTask finished, " + result);
            sActiveTask = null;

            if (result == DownloadManager.STATUS_SUCCESSFUL) {
                Uri uri = mDownloadManager.getUriForDownloadedFile(mDownloadId);
                mListener.onDownloadCompleted(uri);
            } else {
                int error = -1;
                if (result == DownloadManager.STATUS_FAILED) {
                    Query query = new Query().setFilterById(mDownloadId);
                    Cursor cursor = mDownloadManager.query(query);
                    if (cursor != null && cursor.moveToFirst()) {
                        int reasonIdx = cursor.getColumnIndex(DownloadManager.COLUMN_REASON);
                        error = cursor.getInt(reasonIdx);
                    }
                }
                mListener.onDownloadFailed(result, error);
            }
        }
    }
}
