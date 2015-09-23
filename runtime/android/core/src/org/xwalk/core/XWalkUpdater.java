// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.os.Handler;
import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.util.Enumeration;
import java.util.zip.ZipEntry;
import java.util.zip.ZipException;
import java.util.zip.ZipFile;

import junit.framework.Assert;

import org.xwalk.core.XWalkLibraryLoader.DownloadListener;

/**
 * <p><code>XWalkUpdater</code> is a follow-up solution for {@link XWalkInitializer} in case the
 * initialization failed. The users of {@link XWalkActivity} don't need to use this class.</p>
 *
 * <p><code>XWalkUpdater</code> helps to donwload the Crosswalk runtime and displays dialogs to
 * interact with the user. By default, it will navigate to the Crosswalk runtime's page on the
 * default application store, subsequent process will be up to the user. If the developer specified
 * the download URL of the Crosswalk runtime, it will launch the download manager to fetch the APK.
 * To specify the download URL, insert a meta-data element named "xwalk_apk_url" inside the
 * application tag in the Android manifest.
 *
 * <pre>
 * &lt;application&gt;
 *     &lt;meta-data android:name="xwalk_apk_url" android:value="http://host/XWalkRuntimeLib.apk" /&gt;
 * &lt;/application&gt;
 * </pre>
 *
 * <p>After the proper Crosswalk runtime is downloaded and installed, the user will return to
 * current activity from the application store or the installer. The developer should check this
 * point and invoke <code>XWalkInitializer.initAsync()</code> again to repeat the initialization
 * process. Please note that from now on, the application will be running in shared mode.</p>
 *
 * <p>For example:</p>
 *
 * <pre>
 * public class MyActivity extends Activity
 *         implements XWalkInitializer.XWalkInitListener, XWalkUpdater.XWalkUpdateListener {
 *     XWalkUpdater mXWalkUpdater;
 *
 *     &#64;Override
 *     protected void onResume() {
 *         super.onResume();
 *
 *         // Try to initialize again when the user completed update and returned to current
 *         // activity. The initAsync() will do nothing if the initialization has already been
 *         // completed successfully or previous update dialog is still being displayed.
 *         mXWalkInitializer.initAsync();
 *     }
 *
 *     &#64;Override
 *     public void onXWalkInitFailed() {
 *         if (mXWalkUpdater == null) mXWalkUpdater = new mXWalkUpdater(this, this);
 *         mXWalkUpdater.updateXWalkRuntime();
 *     }
 *
 *     ......
 *
 *     &#64;Override
 *     public void onXWalkUpdateStarted() {
 *         // Download manager started to download the APK file.
 *         // Nothing particular to do here.
 *     }
 *
 *     &#64;Override
 *     public void onXWalkUpdateCancelled() {
 *         // The user clicked the "Cancel" button during download.
 *         // Perform error handling here.
 *     }
 *
 *     &#64;Override
 *     public void onXWalkUpdateFailed() {
 *         // There will be an alert dialog to guide the user.
 *         // Nothing particular to do here.
 *     }
 *
 *     &#64;Override
 *     public void onXWalkUpdateCompleted() {
 *         // The app will jump into the installer.
 *         // Nothing particular to do here.
 *     }
 * }
 * </pre>
 *
 * <p>To download the Crosswalk runtime, you need to grant following permissions in the
 * Android manifest:</p>
 *
 * <pre>
 * &lt;uses-permission android:name="android.permission.ACCESS_NETWORK_STATE" /&gt;
 * &lt;uses-permission android:name="android.permission.ACCESS_WIFI_STATE" /&gt;
 * &lt;uses-permission android:name="android.permission.INTERNET" /&gt;
 * &lt;uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" /&gt;
 * </pre>
 *
 *
 * <p>If you grant following permission further, the download doesn't show in the notifications.</p>
 *
 * <pre>
 * &lt;uses-permission android:name="android.permission.DOWNLOAD_WITHOUT_NOTIFICATION" /&gt;
 * </pre>
 *
 * <p><code>There is another experimental way to update the Crosswalk runtime. The app is still
 * bundled with xwalk_shared_library and the Crosswalk runtime APK must be specified via meta-data
 * "xwalk_apk_url". At the first startup, the Crosswalk runtime APK will be downloaded in background
 * silently without interrupting foreground interactions and the downloaded APK will not be installed
 * as the normal application. Instead, the downloaded runtime APK will be used as a plugin. To enable
 * this approach, you need to insert a meta-data named "xwalk_silent_download" into the application tag
 * in the AndroidManifest.xml</p>
 *
 * <p>For example:</p>
 *
 * <pre>
 * &lt;application&gt;
 *     &lt;meta-data android:name="xwalk_silent_download" android:value="enable" /&gt;
 * &lt;/application&gt;
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

        public void onXWalkUpdateStarted();
        public void onXWalkUpdateFailed();
        public void onXWalkUpdateCompleted();
    }

    private static final String XWALK_APK_MARKET_URL = "market://details?id=org.xwalk.core";

    private static final String META_XWALK_APK_URL = "xwalk_apk_url";
    private static final String META_XWALK_SILENT_DOWNLOAD = "xwalk_silent_download";

    private static final String PRIVATE_LIB_DIR = "xwalkcorelib";

    private static final String[] XWALK_LIB_RESOURCES = {
        "libxwalkcore.so",
        "icudtl.dat",
        "xwalk.pak",
        "classes.dex",
    };
    private static final String[] XWALK_LIB_APK = { "XWalkRuntimeLib.apk" };

    private static final String TAG = "XWalkLib";

    private XWalkUpdateListener mUpdateListener;
    private Activity mActivity;
    private XWalkDialogManager mDialogManager;
    private boolean mIsDownloading;
    private String mXWalkApkUrl;
    private Runnable mDownloadCommand;
    private Runnable mCancelCommand;

    /**
     * Create XWalkUpdater for single activity
     *
     * @param listener The {@link XWalkUpdateListener} to use
     * @param activity The activity which initiate the update
     */
    public XWalkUpdater(XWalkUpdateListener listener, Activity activity) {
        this(listener, activity, new XWalkDialogManager(activity));
    }

    XWalkUpdater(XWalkUpdateListener listener, Activity activity, XWalkDialogManager manager) {
        mUpdateListener = listener;
        mActivity = activity;
        mDialogManager = manager;

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
     * Dismiss the dialog showing and waiting for user's input.
     *
     * @return Return false if no dialog is being displayed, true if dismissed the showing dialog.
     */
    public boolean dismissDialog() {
        if (!mDialogManager.isShowingDialog()) return false;
        mDialogManager.dismissDialog();
        return true;
    }

    /**
     * Set the download URL of the Crosswalk runtime. By default, the updater will get the URL from
     * the Android manifest.
     *
     * @param url The download URL.
     */
    public void setXWalkApkUrl(String url) {
        mXWalkApkUrl = url;
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
        if (mIsDownloading || mDialogManager.isShowingDialog()) return false;

        int status = XWalkLibraryLoader.getLibraryStatus();
        if (status == XWalkLibraryInterface.STATUS_PENDING ||
                status == XWalkLibraryInterface.STATUS_MATCH) return false;

        if (isDownloadMode()) {
            if (mXWalkApkUrl == null) {
                mXWalkApkUrl = getAppMetaData(META_XWALK_APK_URL);
                if (mXWalkApkUrl == null) {
                    Log.e(TAG, "Please specify xwalk_apk_url in meta-data");
                    Assert.fail();
                }
                Log.d(TAG, "Crosswalk APK download URL: " + mXWalkApkUrl);
            }
            XWalkLibraryLoader.startDownload(new BackgroundListener(), mActivity, mXWalkApkUrl);
        } else {
            Log.d(TAG, "Update the Crosswalk runtime with status " + status);
            mDialogManager.showInitializationError(status, mCancelCommand, mDownloadCommand);
        }
        return true;
    }

    private boolean isDownloadMode() {
        String silentDownload = getAppMetaData(META_XWALK_SILENT_DOWNLOAD);
        if (silentDownload != null && silentDownload.equalsIgnoreCase("enable")) {
            Log.i(TAG, "Running in silent download mode");
            return true;
        }
        return false;
    }

    private void downloadXWalkApk() {
        // The download url is defined by the meta-data element with the name "xwalk_apk_url"
        // inside the application tag in the Android manifest. It can also be specified via
        // the option --xwalk-apk-url of the script make_apk.
        if (mXWalkApkUrl == null) {
            mXWalkApkUrl = getAppMetaData(META_XWALK_APK_URL);
            if (mXWalkApkUrl == null) mXWalkApkUrl = "";
            Log.d(TAG, "Crosswalk APK download URL: " + mXWalkApkUrl);
        }

        if (!mXWalkApkUrl.isEmpty()) {
            XWalkLibraryLoader.startDownload(new ForegroundListener(), mActivity, mXWalkApkUrl);
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

    /**
     * Cancel the download of the Crosswalk library.
     *
     * @return True if the download is canclled, false otherwise.
     */
    public boolean cancelLibraryUpdate() {
        return XWalkLibraryLoader.cancelDownload();
    }

    private boolean installLibraries(String fileName) {
        String libDir = mActivity.getDir(PRIVATE_LIB_DIR, Context.MODE_PRIVATE).toString();
        try {
            ZipFile zipFile = new ZipFile(fileName);
            for (String resource : XWALK_LIB_RESOURCES) {
                String entryName;
                String dest = libDir + File.separator + resource;
                if (resource.substring(resource.length() - 2).equals("so")) {
                    if(Build.CPU_ABI.equalsIgnoreCase("armeabi")) {
                        // We build armeabi-v7a native lib for both armeabi & armeabi-v7a
                        entryName = "lib" + File.separator + "armeabi-v7a" +
                                File.separator + resource;
                    } else {
                        entryName = "lib" + File.separator + Build.CPU_ABI +
                                File.separator + resource;
                    }
                } else if (resource.substring(resource.length() - 3).equals("dat") ||
                        resource.substring(resource.length() - 3).equals("pak")) {
                    entryName = "assets" + File.separator + resource;
                } else {
                    entryName = resource;
                }
                Log.d(TAG, "unzip " + entryName);
                ZipEntry entry = zipFile.getEntry(entryName);
                copy(zipFile.getInputStream(entry), new FileOutputStream(dest));
            }
        } catch (IOException e) {
            Log.d(TAG, e.getLocalizedMessage());
            return false;
        }

        return true;
    }

    private class ForegroundListener implements DownloadListener {
        @Override
        public void onDownloadStarted() {
            mDialogManager.showDownloadProgress(new Runnable() {
                @Override
                public void run() {
                    XWalkLibraryLoader.cancelDownload();
                }
            });

            mUpdateListener.onXWalkUpdateStarted();
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
        public void onDownloadFailed(int status, int error) {
            mDialogManager.dismissDialog();
            mDialogManager.showDownloadError(status, error, mCancelCommand, mDownloadCommand);
            mUpdateListener.onXWalkUpdateFailed();
        }

        @Override
        public void onDownloadCompleted(Uri uri) {
            mDialogManager.dismissDialog();
            mUpdateListener.onXWalkUpdateCompleted();

            Log.d(TAG, "Install the Crosswalk runtime: " + uri.toString());
            Intent install = new Intent(Intent.ACTION_VIEW);
            install.setDataAndType(uri, "application/vnd.android.package-archive");
            mActivity.startActivity(install);
        }
    }

    private class BackgroundListener implements DownloadListener {
        @Override
        public void onDownloadStarted() {
            mIsDownloading = true;
            mUpdateListener.onXWalkUpdateStarted();
        }

        @Override
        public void onDownloadUpdated(int percentage) {
        }

        @Override
        public void onDownloadCancelled() {
            mIsDownloading = false;
            mUpdateListener.onXWalkUpdateCancelled();
        }

        @Override
        public void onDownloadFailed(int status, int error) {
            mIsDownloading = false;
            mUpdateListener.onXWalkUpdateFailed();
        }

        @Override
        public void onDownloadCompleted(Uri uri) {
            if(!installLibraries(uri.getPath())) Assert.fail();

            mIsDownloading = false;
            mUpdateListener.onXWalkUpdateCompleted();
        }
    }

    private String getAppMetaData(String name) {
        try {
            PackageManager packageManager = mActivity.getPackageManager();
            ApplicationInfo appInfo = packageManager.getApplicationInfo(
                    mActivity.getPackageName(), PackageManager.GET_META_DATA);
            return appInfo.metaData.getString(name);
        } catch (NameNotFoundException | NullPointerException e) {
        }
        return null;
    }

    private void copy(InputStream input, OutputStream output) throws IOException {
        byte[] buffer = new byte[4096];
        int len = 0;
        while ((len = input.read(buffer)) >= 0) {
            output.write(buffer, 0, len);
        }

        input.close();
        output.close();
    }
}
