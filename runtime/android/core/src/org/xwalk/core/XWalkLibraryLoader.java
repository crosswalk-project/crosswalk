// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

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

import org.xwalk.core.XWalkLibraryInterface.DecompressionListener;
import org.xwalk.core.XWalkLibraryInterface.DownloadListener;
import org.xwalk.core.XWalkLibraryInterface.InitializationListener;

/**
 * <code>XWalkLibraryLoader</code> is a low level inteface to schedule decompressing,
 * downloading, initializing the Crosswalk Library. Normal user is recommended to use
 * {@link XWalkActivity}(with UI) or {@link XWalkInitializer}(without UI) which is more
 * simple and more user friendly.
 *
 * <p>The appropriate invocation order is:
 * prepareToInit() -
 * [ if native library is supposed to be compressed - startDecompression() ] -
 * initXWalkLibrary() -
 * [ if the library doesn't match - install suitable library - initXWalkLibrary() ] -
 * startInitialization() - over
 */
class XWalkLibraryLoader {
    private static final String XWALK_APK_NAME = "XWalkRuntimeLib.apk";
    private static final String TAG = "XWalkLib";

    private static AsyncTask<Void, Integer, Integer> sActiveTask;
    private static boolean sPreparedToInit = false;

    /**
     * Returns true if running in shared mode, false if running in embedded mode.
     *
     * <p>This method must be invoked after initXWalkLibrary returns STATUS_MATCH
     */
    public static boolean isSharedMode() {
        return XWalkCoreWrapper.getInstance().isSharedMode();
    }

    /**
     * Prepares to start initialization.
     *
     * <p>This method must be invoked on the UI thread and before other procedures
     */
    public static void prepareToInit() {
        if (sPreparedToInit) return;
        sPreparedToInit = true;
        XWalkCoreWrapper.handlePreInit();
    }

    /**
     * Links to the Crosswalk library either is embedded in current application or is shared
     * across package
     *
     * <p>This method must be invoked on the UI thread.
     *
     * @return The library status that defined in {@link XWalkLibrarayInterface}
     */
    public static int initXWalkLibrary(Context context) {
        if (!sPreparedToInit) Assert.fail("Invoke prepareToInit first");
        return XWalkCoreWrapper.initXWalkLibrary(context);
    }

    /**
     * Starts initializing the Crosswalk environment in background. The initialization is not
     * cancelable.
     *
     * <p>This method must be invoked on the UI thread.
     *
     * @param listener The {@link XWalkLibraryInterface.InitializationListener} to use
     */
    public static void startInitialization(InitializationListener listener) {
        if (XWalkCoreWrapper.getInstance() == null) Assert.fail("Invoke initXWalkLibrary first");
        new InitializationTask(listener).execute();
    }

    /**
     * Starts decompressing native library in background
     *
     * <p>This method must be invoked on the UI thread.
     *
     * @param listener The {@link XWalkLibraryInterface.DecompressionListener} to use
     * @param context The context of the package that holds the compressed native library
     *
     * @return False if native library is not compressed, true otherwise
     */
    public static boolean startDecompression(DecompressionListener listener, Context context) {
        return new DecompressionTask(listener, context).executeIfNeeded();
    }

    /**
     * Attempts to cancel decompression
     *
     * @return False if decompression is not running or could not be cancelled, true otherwise
     */
    public static boolean cancelDecompression() {
        DecompressionTask task = (DecompressionTask) sActiveTask;
        return task != null && task.cancel(true);
    }

    /**
     * Starts downloading Crosswalk library in background via Android DownlomadManager
     *
     * <p>This method must be invoked on the UI thread.
     *
     * @param listener The {@link XWalkLibraryInterface.InitializationListener} to use
     * @param context The context to get DownloadManager
     * @param url The URL of the Crosswalk library
     */
    public static void startDownload(DownloadListener listener, Context context, String url) {
        new DownloadTask(listener, context, url).execute();
    }

    /**
     * Attempts to cancel download
     *
     * @return False if download is not running or could not be cancelled, true otherwise
     */
    public static boolean cancelDownload() {
        DownloadTask task = (DownloadTask) sActiveTask;
        return task != null && task.cancel(true);
    }

    private static class InitializationTask extends AsyncTask<Void, Integer, Integer> {
        InitializationListener mListener;

        InitializationTask(InitializationListener listener) {
            super();
            mListener = listener;
        }

        @Override
        protected void onPreExecute() {
            Log.d(TAG, "InitializationTask started");
            sActiveTask = this;
            mListener.onInitializationStarted();
        }

        @Override
        protected Integer doInBackground(Void... params) {
            XWalkCoreWrapper.initXWalkEnvironment();
            return 0;
        }

        @Override
        protected void onPostExecute(Integer result) {
            Log.d(TAG, "InitializationTask finished");
            sActiveTask = null;
            XWalkCoreWrapper.handlePostInit();
            mListener.onInitializationCompleted();
        }
    }

    private static class DecompressionTask extends AsyncTask<Void, Integer, Integer> {
        DecompressionListener mListener;
        Context mContext;

        DecompressionTask(DecompressionListener listener, Context context) {
            super();
            mListener = listener;
            mContext = context;
        }

        public boolean executeIfNeeded() {
            if (!XWalkLibraryDecompressor.isCompressed(mContext)) return false;
            Log.d(TAG, "Using compressed native library");
            execute();
            return true;
        }

        @Override
        protected void onPreExecute() {
            Log.d(TAG, "DecompressionTask started");
            sActiveTask = this;
            mListener.onDecompressionStarted();
        }

        @Override
        protected Integer doInBackground(Void... params) {
            if (!XWalkLibraryDecompressor.isDecompressed(mContext) &&
                    !XWalkLibraryDecompressor.decompressLibrary(mContext)) {
                Assert.fail();
            }

            XWalkLibraryDecompressor.loadDecompressedLibrary(mContext);
            return 0;
        }

        @Override
        protected void onCancelled(Integer result) {
            Log.d(TAG, "DecompressionTask cancelled");
            sActiveTask = null;
            mListener.onDecompressionCancelled();
        }

        @Override
        protected void onPostExecute(Integer result) {
            Log.d(TAG, "DecompressionTask finished");
            sActiveTask = null;
            mListener.onDecompressionCompleted();
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
            Log.d(TAG, "DownloadTask cancelled");
            sActiveTask = null;
            mDownloadManager.remove(mDownloadId);
            mListener.onDownloadCancelled();
        }

        @Override
        protected void onPostExecute(Integer result) {
            Log.d(TAG, "DownloadTask finished, " + result);
            sActiveTask = null;

            if (result == DownloadManager.STATUS_SUCCESSFUL) {
                Uri uri = mDownloadManager.getUriForDownloadedFile(mDownloadId);
                mListener.onDownloadCompleted(uri);
                return;
            }

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
