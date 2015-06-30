// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.net.Uri;
import android.util.Log;

import org.xwalk.core.XWalkLibraryLoader.DownloadListener;

/**
 * <p><code>XWalkUpdater</code> is a follow-up solution for {@link XWalkInitializer} in case the
 * initialization has failed. The {@link XWalkActivity}'s users don't need to use this class.</p>
 *
 * <p><code>XWalkUpdater</code> helps to update the Crosswalk runtime and displays dialogs to
 * interact with the user. After the Crosswalk runtime is downloaded and installed properly,
 * the user will return to current activity from the play store or the installer. You should check
 * this point and invoke <code>XWalkInitializer.initAsync()</code> again to repeat the
 * initialization process. Please note that from now on, the application will be running in shared
 * mode.</p>
 *
 * <p>For example:</p>
 *
 * <pre>
 * public class MyActivity extends Activity
 *         implements XWalkInitializer.XWalkInitListener, XWalkUpdater.XWalkUpdateListener {
 *     XWalkUpdater mXWalkUpdater;
 *
 *     ......
 *
 *     &#64;Override
 *     protected void onResume() {
 *         super.onResume();
 *
 *         // Try to initialize again when the user completed updating and returned to current
 *         // activity. The initAsync() will do nothing if the initialization has already been
 *         // completed successfully.
 *         mXWalkInitializer.initAsync();
 *     }
 *
 *     &#64;Override
 *     public void onXWalkInitFailed() {
 *         if (mXWalkUpdater == null) mXWalkUpdater = new mXWalkUpdater(this, this);
 *
 *         // The updater won't be launched if previous update dialog is showing.
 *         mXWalkUpdater.updateXWalkRuntime();
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
         * Run on the UI thread to notify the update is cancelled. It could be the user refused to
         * update or the download (from the specified URL) is cancelled
         */
        public void onXWalkUpdateCancelled();
    }

    private static final String XWALK_APK_MARKET_URL = "market://details?id=org.xwalk.core";
    private static final String TAG = "XWalkActivity";

    private XWalkUpdateListener mUpdateListener;
    private Activity mActivity;
    private XWalkDialogManager mDialogManager;
    private Runnable mDownloadCommand;
    private Runnable mCancelCommand;
    private String mXWalkApkUrl;

    /**
     * Create XWalkUpdater for single activity
     *
     * @param listener The {@link XWalkUpdateListener} to use
     * @param activity The activity which initiate the update
     */
    public XWalkUpdater(XWalkUpdateListener listener, Activity activity) {
        this(listener, activity, new XWalkDialogManager(activity));
    }

    XWalkUpdater(XWalkUpdateListener listener, Activity activity,
            XWalkDialogManager dialogManager) {
        mUpdateListener = listener;
        mActivity = activity;
        mDialogManager = dialogManager;

        mDownloadCommand = new Runnable() {
            @Override
            public void run() {
                downloadXWalkApk();
            }
        };
        mCancelCommand = new Runnable() {
            @Override
            public void run() {
                Log.d(TAG, "XWalkUpdater cancelled");
                mUpdateListener.onXWalkUpdateCancelled();
            }
        };
    }

    /**
     * Update the Crosswalk runtime. There are 2 ways to download the Crosswalk runtime: from the
     * play store or the specified URL. It will download from the play store if the download URL is
     * not specified. To specify the download URL, insert a meta-data element with the name
     * "xwalk_apk_url" inside the application tag in the Android manifest.
     *
     * <p>Please try to initialize by {@link XWalkInitializer} first and only invoke this method
     * when the initialization failed. This method must be invoked on the UI thread.
     *
     * @return True if the updater is launched, false if is in updating, or the Crosswalk runtime
     *         doesn't need to be updated, or the Crosswalk runtime has not been initialized yet.
     */
    public boolean updateXWalkRuntime() {
        if (mDialogManager.isShowingDialog()) return false;

        int status = XWalkLibraryLoader.getLibraryStatus();
        if (status == XWalkLibraryInterface.STATUS_PENDING ||
                status == XWalkLibraryInterface.STATUS_MATCH) return false;

        Log.d(TAG, "Update the Crosswalk runtime with status " + status);
        mDialogManager.showInitializationError(status, mCancelCommand, mDownloadCommand);
        return true;
    }

    private void downloadXWalkApk() {
        String downloadUrl = getXWalkApkUrl();
        if (!downloadUrl.isEmpty()) {
            XWalkLibraryLoader.startDownload(new XWalkLibraryListener(), mActivity, downloadUrl);
            return;
        }

        try {
            Intent intent = new Intent(Intent.ACTION_VIEW);
            mActivity.startActivity(intent.setData(Uri.parse(XWALK_APK_MARKET_URL)));

            Log.d(TAG, "Market opened");
            mDialogManager.dismissDialog();
        } catch (ActivityNotFoundException e) {
            Log.d(TAG, "Market open failed");
            mDialogManager.showMarketOpenError(mCancelCommand);
        }
    }

    protected String getXWalkApkUrl() {
        if (mXWalkApkUrl != null) return mXWalkApkUrl;

        // The download url is defined by the meta-data element with the name "xwalk_apk_url"
        // inside the application tag in the Android manifest. It can also be specified via
        // the option --xwalk-apk-url of the script make_apk.
        try {
            PackageManager packageManager = mActivity.getPackageManager();
            ApplicationInfo appInfo = packageManager.getApplicationInfo(
                    mActivity.getPackageName(), PackageManager.GET_META_DATA);
            mXWalkApkUrl = appInfo.metaData.getString("xwalk_apk_url");
        } catch (NameNotFoundException | NullPointerException e) {
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
            mUpdateListener.onXWalkUpdateCancelled();
        }

        @Override
        public void onDownloadCompleted(Uri uri) {
            mDialogManager.dismissDialog();

            Log.d(TAG, "Install the Crosswalk runtime: " + uri.toString());
            Intent install = new Intent(Intent.ACTION_VIEW);
            install.setDataAndType(uri, "application/vnd.android.package-archive");
            mActivity.startActivity(install);
        }

        @Override
        public void onDownloadFailed(int status, int error) {
            mDialogManager.dismissDialog();
            mDialogManager.showDownloadError(status, error, mCancelCommand, mDownloadCommand);
        }
    }
}
