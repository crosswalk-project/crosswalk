// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.net.Uri;
import android.util.Log;

import org.xwalk.core.XWalkLibraryLoader.DownloadListener;

/**
 * <p><code>XWalkUpdater</code> helps to update the Crosswalk runtime and displays dialogs to
 * interact with the user. After the Crosswalk runtime is downloaded and installed properly,
 * the user will return to current activity from the play store or the installer. You should check
 * this point and invoke <code>XWalkInitializer.initAsync()</code> again to repeat the installation
 * process. Please note that from now on, the application will be running in shared mode.</p>
 *
 * <p>For example:</p>
 *
 * <pre>
 * public class MyActivity extends Activity
 *         implements XWalkInitializer.XWalkInitListener, XWalkUpdater.XWalkUpdateListener {
 *
 *     ......
 *
 *     &#64;Override
 *     protected void onResume() {
 *         super.onResume();
 *
 *         if (mIsXWalkReady) XWalkInitializer.initAsync(this, this);
 *     }
 *
 *     &#64;Override
 *     public void onXWalkInitCompleted() {
 *         mIsXWalkReady = true;
 *
 *         // Do anyting with the embedding API
 *     }
 *
 *     &#64;Override
 *     public void onXWalkInitFailed() {
 *         XWalkUpdater.updateXWalkRuntime(this, this);
 *     }
 *
 *     &#64;Override
 *     public void onXWalkUpdateCancelled() {
 *         // Perform error handling here
 *     }
 * }
 * </pre>
 */

public class XWalkUpdater {
    /**
     * Interface used to update the Crosswalk runtime
     */
    public interface XWalkUpdateListener {
        /**
         * Runs on the UI thread to notify update is cancelled. It could be the user refused to
         * update or the download (from the specified URL) is cancelled
         */
        public void onXWalkUpdateCancelled();
    }

    private static XWalkUpdater sInstance;

    /**
     * Update the Crosswalk runtime. There are 2 ways to download the Crosswalk runtime: from the
     * play store or the specified URL. The download URL is defined by the meta-data element with
     * the name "xwalk_apk_url" enclosed the application tag in the Android manifest. If the download
     * URL is not defined, it will download from the play store.
     *
     * <p>This method must be invoked on the UI thread.
     *
     * @param listener The {@link XWalkUpdateListener} to use
     * @param context The context that update is to run it
     *
     * @return False if is in updating or the Crosswalk runtime doesn't need to be updated,
     *         true otherwise.
     */
    public static boolean updateXWalkRuntime(XWalkUpdateListener listener, Context context) {
        if (sInstance != null) return false;
        return new XWalkUpdater(listener, context).tryUpdate();
    }

    /**
     * Check whether the dialog is being displayed and waiting for user's input
     */
    public static boolean isWaitingForInput() {
        return sInstance != null && sInstance.mDialogManager.isShowingDialog();
    }

    /**
     * Dismiss the dialog which is waiting for user's input. Please check with isWaitingForInput()
     * before invoking this method.
     */
    public static void dismissDialog() {
        sInstance.mDialogManager.dismissDialog();
    }

    private static final String XWALK_APK_MARKET_URL = "market://details?id=org.xwalk.core";
    private static final String TAG = "XWalkLib";

    private XWalkUpdateListener mUpdateListener;
    private Context mContext;
    private XWalkDialogManager mDialogManager;
    private Runnable mDownloadCommand;
    private Runnable mCancelCommand;
    private String mXWalkApkUrl;

    private XWalkUpdater(XWalkUpdateListener listener, Context context) {
        mUpdateListener = listener;
        mContext = context;
        mDialogManager = new XWalkDialogManager(context);

        mDownloadCommand = new Runnable() {
            @Override
            public void run() {
                downloadXWalkLibrary();
            }
        };
        mCancelCommand = new Runnable() {
            @Override
            public void run() {
                sInstance = null;
                mUpdateListener.onXWalkUpdateCancelled();
            }
        };

        XWalkLibraryLoader.prepareToInit();
    }

    private boolean tryUpdate() {
        if (XWalkCoreWrapper.getInstance() != null) return false;

        sInstance = this;
        int status = XWalkCoreWrapper.getProvisionalStatus();
        mDialogManager.showInitializationError(status, mCancelCommand, mDownloadCommand);
        return true;
    }

    private void downloadXWalkLibrary() {
        String downloadUrl = getXWalkApkUrl();
        if (!downloadUrl.isEmpty()) {
            XWalkLibraryLoader.startDownload(new XWalkLibraryListener(), mContext, downloadUrl);
            return;
        }

        try {
            Intent intent = new Intent(Intent.ACTION_VIEW);
            mContext.startActivity(intent.setData(Uri.parse(XWALK_APK_MARKET_URL)));

            mDialogManager.dismissDialog();
            sInstance = null;
        } catch (ActivityNotFoundException e) {
            Log.d(TAG, "Market open failed");
            mDialogManager.showMarketOpenError(mCancelCommand);
        }
    }

    protected String getXWalkApkUrl() {
        if (mXWalkApkUrl != null) return mXWalkApkUrl;

        // The download url is defined by the meta-data element with the name "xwalk_apk_url"
        // inside the application tag in the Android manifest. It can also be specified via
        // --xwalk-apk-url option of make_apk script indirectly.
        try {
            PackageManager packageManager = mContext.getPackageManager();
            ApplicationInfo appInfo = packageManager.getApplicationInfo(
                mContext.getPackageName(), PackageManager.GET_META_DATA);
            if (appInfo.metaData != null) {
                mXWalkApkUrl = appInfo.metaData.getString("xwalk_apk_url");
            }
        } catch (NameNotFoundException e) {
        }

        if (mXWalkApkUrl == null) mXWalkApkUrl = "";
        Log.d(TAG, "Crosswalk APK download URL: " + mXWalkApkUrl);
        return mXWalkApkUrl;
    }

    private class XWalkLibraryListener implements DownloadListener {
        @Override
        public void onDownloadStarted() {
            mDialogManager.showDownloadProgress(new Runnable() {
                @Override
                public void run() {
                    XWalkLibraryLoader.cancelDownload();
                }
            });
        }

        @Override
        public void onDownloadUpdated(int percentage) {
            mDialogManager.setProgress(percentage, 100);
        }

        @Override
        public void onDownloadCancelled() {
            sInstance = null;
            mUpdateListener.onXWalkUpdateCancelled();
        }

        @Override
        public void onDownloadCompleted(Uri uri) {
            mDialogManager.dismissDialog();
            sInstance = null;

            Log.d(TAG, "Install the Crosswalk library, " + uri.toString());
            Intent install = new Intent(Intent.ACTION_VIEW);
            install.setDataAndType(uri, "application/vnd.android.package-archive");
            mContext.startActivity(install);
        }

        @Override
        public void onDownloadFailed(int status, int error) {
            mDialogManager.dismissDialog();
            mDialogManager.showDownloadError(status, error, mCancelCommand, mDownloadCommand);
        }
    }
}
