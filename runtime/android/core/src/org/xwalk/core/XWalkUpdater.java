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
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Environment;
import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.io.OutputStream;
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
 * <p><code>XWalkUpdater</code> helps to download the Crosswalk runtime and displays dialogs to
 * interact with the user. By default, it will navigate to the Crosswalk runtime's page on the
 * default application store, subsequent process will be up to the user. If the developer specified
 * the download URL of the Crosswalk runtime, it will launch the download manager to fetch the APK.
 * To specify the download URL, insert a meta-data element named "xwalk_apk_url" inside the
 * application tag in the Android manifest.</p>
 *
 * <pre>
 * &lt;application&gt;
 *     &lt;meta-data android:name="xwalk_apk_url" android:value="http://host/XWalkRuntimeLib.apk" /&gt;
 * &lt;/application&gt;
 * </pre>
 *
 * <p>To download the Crosswalk runtime APK for the specified CPU architecture, <code>xwalk_apk_url</code>
 * will be appended with a query string named "?arch=CPU_ABI" when the download request is sent to server,
 * then server can send back the APK which is exactly built for the specified CPU architecture. The CPU_ABI
 * here is exactly same as the value returned from "getprop ro.product.cpu.abi".</p>
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
 *     ......
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
 *
 *         // The updater won't be launched if previous update dialog is showing.
 *         mXWalkUpdater.updateXWalkRuntime();
 *     }
 *
 *     &#64;Override
 *     public void onXWalkUpdateCancelled() {
 *         // The user clicked the "Cancel" button during download.
 *         // Perform error handling here.
 *     }
 * }
 * </pre>
 *
 * <p>To download the Crosswalk runtime, you need to grant following permissions in the
 * Android manifest:</p>
 *
 * <pre>
 * &lt;uses-permission android:name="android.permission.ACCESS_NETWORK_STATE" /&gt;
 * &lt;uses-permission android:name="android.permission.CHANGE_NETWORK_STATE" /&gt;
 * &lt;uses-permission android:name="android.permission.ACCESS_WIFI_STATE" /&gt;
 * &lt;uses-permission android:name="android.permission.CHANGE_WIFI_STATE" /&gt;
 * &lt;uses-permission android:name="android.permission.INTERNET" /&gt;
 * &lt;uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" /&gt;
 * </pre>
 *
 * <p>There is an experimental way to update the Crosswalk runtime. All of the download process are
 * executed in background silently without interrupting the user.</p>
 *
 * <p>For example:</p>
 *
 * <pre>
 * public class MyActivity extends Activity
 *         implements XWalkInitializer.XWalkInitListener, XWalkUpdater.XWalkBackgroundUpdateListener {
 *     XWalkUpdater mXWalkUpdater;
 *
 *     ......
 *
 *     &#64;Override
 *     protected void onResume() {
 *         super.onResume();
 *     }
 *
 *     &#64;Override
 *     public void onXWalkInitFailed() {
 *         if (mXWalkUpdater == null) mXWalkUpdater = new mXWalkUpdater(this, this);
 *         mXWalkUpdater.updateXWalkLibrary();
 *     }
 *
 *     &#64;Override
 *     public void onXWalkUpdateStarted() {
 *         // Download started in background
 *         // Nothing particular to do here.
 *     }
 *
 *     &#64;Override
 *     public void onXWalkUpdateProgress(int percentage) {
 *         // Update the progress of download
 *         // Nothing particular to do here.
 *     }
 *
 *     &#64;Override
 *     public void onXWalkUpdateCancelled() {
 *         // Background download is cancelled by invoking cancelBackgroundDownload().
 *         // Perform error handling here
 *     }
 *
 *     &#64;Override
 *     public void onXWalkUpdateFailed() {
 *         // Background download failed.
 *         // Perform error handling here
 *     }
 *
 *     &#64;Override
 *     public void onXWalkUpdateCompleted() {
 *         // Try to initialize again when the Crosswalk libraries are ready.
 *         mXWalkInitializer.initAsync();
 *     }
 * }
 * </pre>
 *
 * <p>If you grant following permission further, the download doesn't show in the notifications.</p>
 *
 * <pre>
 * &lt;uses-permission android:name="android.permission.DOWNLOAD_WITHOUT_NOTIFICATION" /&gt;
 * </pre>
 *
 * <p>Signature check is enabled by default in download mode, it requires the crosswalk runtime APK
 * must be signed with the same key used in application APK signing. Developer can insert a
 * meta-data "xwalk_verify" in AndroidManifest to disable the signature check.</p>
 *
 * <pre>
 * &lt;application&gt;
 *     &lt;meta-data android:name="xwalk_verify" android:value="disable" /&gt;
 * &lt;/application&gt;
 * </pre>
 *
 * <p>We have crosswalk runtime auto-update enabled by default in download mode, it requires that
 * the build version of the xwalk_shared_library which you used to bundle with your application
 * has to match the build version of the downloaded crosswalk runtime, if the build versions differ
 * it will trigger an update to download a new crosswalk runtime from the server. If you want to
 * disable auto update, you could call <code>setAutoUpdate(false)</code> before starting to
 * initialization.</p>
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

    /**
     * Interface used to update the Crosswalk runtime silently
     */
    public interface XWalkBackgroundUpdateListener {
        /**
         * Run on the UI thread to notify the update is started.
         */
        public void onXWalkUpdateStarted();

        /**
         * Run on the UI thread to notify the update progress.
         * @param percentage Shows the update progress in percentage.
         */
        public void onXWalkUpdateProgress(int percentage);

        /**
         * Run on the UI thread to notify the update is cancelled.
         */
        public void onXWalkUpdateCancelled();

        /**
         * Run on the UI thread to notify the update failed.
         */
        public void onXWalkUpdateFailed();

        /**
         * Run on the UI thread to notify the update is completed.
         */
        public void onXWalkUpdateCompleted();
    }

    private static final String XWALK_APK_MARKET_URL = "market://details?id=org.xwalk.core";

    private static final String META_XWALK_APK_URL = "xwalk_apk_url";
    private static final String META_XWALK_VERIFY = "xwalk_verify";
    private static final String XWALK_CORE_EXTRACTED_DIR = "extracted_xwalkcore";
    private static final String ARCH_QUERY_STRING = "?arch=";

    private static final String[] XWALK_LIB_RESOURCES = {
        "libxwalkcore.so",
        "classes.dex",
        "icudtl.dat",
        "xwalk.pak",
    };

    private static final String TAG = "XWalkLib";

    private static final int STREAM_BUFFER_SIZE = 0x1000;
    private static boolean sAutoUpdateEnabled = true;

    private XWalkUpdateListener mUpdateListener;
    private XWalkBackgroundUpdateListener mBackgroundUpdateListener;
    private Activity mActivity;
    private XWalkDialogManager mDialogManager;
    private Runnable mDownloadCommand;
    private Runnable mCancelCommand;
    private String mXWalkApkUrl;
    private boolean mIsDownloading;

    /**
     * Create XWalkUpdater for single activity
     *
     * @param listener The {@link XWalkUpdateListener} to use
     * @param activity The activity which initiate the update
     */
    public XWalkUpdater(XWalkUpdateListener listener, Activity activity) {
        mUpdateListener = listener;
        mActivity = activity;
        mDialogManager = new XWalkDialogManager(activity);
    }

    // This constructor is for XWalkActivityDelegate
    XWalkUpdater(XWalkUpdateListener listener, Activity activity,
            XWalkDialogManager dialogManager) {
        mUpdateListener = listener;
        mActivity = activity;
        mDialogManager = dialogManager;
    }

    /**
     * Create XWalkUpdater for single activity. This updater will download silently.
     *
     * @param listener The {@link XWalkBackgroundUpdateListener} to use
     * @param activity The activity which initiate the update
     */
    public XWalkUpdater(XWalkBackgroundUpdateListener listener, Activity activity) {
        mBackgroundUpdateListener = listener;
        mActivity = activity;
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
        if (mIsDownloading || (mDialogManager != null && mDialogManager.isShowingDialog())) {
            return false;
        }

        int status = XWalkLibraryLoader.getLibraryStatus();
        if (status == XWalkLibraryInterface.STATUS_PENDING ||
                status == XWalkLibraryInterface.STATUS_MATCH) return false;

        if (mUpdateListener != null) {
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

            mDialogManager.showInitializationError(status, mCancelCommand, mDownloadCommand);
        } else if (mBackgroundUpdateListener != null) {
            downloadXWalkApkSilently();
        } else {
            Assert.fail();
        }

        return true;
    }

    /**
     * Dismiss the dialog showing and waiting for user's input.
     *
     * @return Return false if no dialog is being displayed, true if dismissed the showing dialog.
     */
    public boolean dismissDialog() {
        if (mDialogManager == null || !mDialogManager.isShowingDialog()) return false;
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
     * Cancel the background download
     *
     * @return Return false if it is not a background updater or is not downloading, true otherwise.
     */
    public boolean cancelBackgroundDownload() {
        if (mBackgroundUpdateListener == null || !mIsDownloading) return false;
        return XWalkLibraryLoader.cancelDownload();
    }

    /**
     * Enable/disable crosswak runtime auto update in download mode. It's enabled by default.
     * @param enable True to enable auto update, otherwise disable it.
     */
    public static void setAutoUpdate(boolean enable) {
        sAutoUpdateEnabled = enable;
    }

    /**
     * Get the crosswalk runtime auto update status in download mode.
     * @return Return true if auto update is enabled, otherwise it's disabled.
     */
    public static boolean getAutoUpdate() {
        return sAutoUpdateEnabled;
    }

    private void downloadXWalkApk() {
        // The download url is defined by the meta-data element with the name "xwalk_apk_url"
        // inside the application tag in the Android manifest. It can also be specified via
        // the option --xwalk-apk-url of the script make_apk.
        if (mXWalkApkUrl == null) {
            mXWalkApkUrl = getXWalkApkUrl();
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

    private void downloadXWalkApkSilently() {
        if (mXWalkApkUrl == null) {
            mXWalkApkUrl = getXWalkApkUrl();
            Log.d(TAG, "Crosswalk APK download URL: " + mXWalkApkUrl);
        }
        XWalkLibraryLoader.startDownload(new BackgroundListener(), mActivity, mXWalkApkUrl);
    }

    private String getXWalkApkUrl() {
        String url = getAppMetaData(META_XWALK_APK_URL);
        return url == null ? "" : url + ARCH_QUERY_STRING + Build.CPU_ABI;
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
        }

        @Override
        public void onDownloadCompleted(Uri uri) {
            mDialogManager.dismissDialog();

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
            mBackgroundUpdateListener.onXWalkUpdateStarted();
        }

        @Override
        public void onDownloadUpdated(int percentage) {
            mBackgroundUpdateListener.onXWalkUpdateProgress(percentage);
        }

        @Override
        public void onDownloadCancelled() {
            mIsDownloading = false;
            mBackgroundUpdateListener.onXWalkUpdateCancelled();
        }

        @Override
        public void onDownloadFailed(int status, int error) {
            mIsDownloading = false;
            mBackgroundUpdateListener.onXWalkUpdateFailed();
        }

        @Override
        public void onDownloadCompleted(Uri uri) {
            mIsDownloading = false;

            final Uri downloadedUri = uri;
            new AsyncTask<Void, Void, Void>() {
                @Override
                protected Void doInBackground(Void... params) {
                    final String destDir = mActivity.getDir(XWALK_CORE_EXTRACTED_DIR,
                            Context.MODE_PRIVATE).getAbsolutePath();
                    if (!verifyXWalkRuntimeLib(downloadedUri.getPath())) Assert.fail();
                    if (!extractLibResources(downloadedUri.getPath(), destDir)) Assert.fail();
                    XWalkCoreWrapper.resetXWalkRuntimeBuildVersion(mActivity);
                    return null;
                }

                @Override
                protected void onPostExecute(Void result) {
                    mBackgroundUpdateListener.onXWalkUpdateCompleted();
                }
            }.execute();
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

    private boolean checkSignature(PackageInfo runtimePackageInfo, PackageInfo appPackageInfo) {
        if (runtimePackageInfo.signatures == null || appPackageInfo.signatures == null) {
            Log.e(TAG, "No signature in package info");
            return false;
        }

        if (runtimePackageInfo.signatures.length != appPackageInfo.signatures.length) {
            Log.e(TAG, "signatures length not equal");
            return false;
        }

        for (int i = 0; i < runtimePackageInfo.signatures.length; ++i) {
            Log.d(TAG, "Checking signature " + i);
            if (!appPackageInfo.signatures[i].equals(runtimePackageInfo.signatures[i])) {
                Log.e(TAG, "signatures do not match");
                continue;
            }
            Log.d(TAG, "signature check PASSED");
            return true;
        }

        return false;
    }

    private boolean verifyXWalkRuntimeLib(String libFile) {
        String xwalkVerify = getAppMetaData(META_XWALK_VERIFY);
        if (xwalkVerify != null && xwalkVerify.equals("disable")) {
            Log.w(TAG, "xwalk verify is disabled");
            return true;
        }

        // getPackageArchiveInfo also check the integrity of the downloaded runtime APK
        // besides returning the PackageInfo with signatures.
        PackageInfo runtimePkgInfo = mActivity.getPackageManager().getPackageArchiveInfo(
                libFile, PackageManager.GET_SIGNATURES);
        if (runtimePkgInfo == null) {
            Log.e(TAG, "The downloaded XWalkRuntimeLib.apk is invalid!");
            return false;
        }
        PackageInfo appPkgInfo = null;
        try {
            appPkgInfo = mActivity.getPackageManager().getPackageInfo(
                    mActivity.getPackageName(), PackageManager.GET_SIGNATURES);
        } catch (PackageManager.NameNotFoundException e) {
            return false;
        }

        // Verify if App APK and Runtime APK were signed with the same key
        return checkSignature(runtimePkgInfo, appPkgInfo);
    }

    private boolean extractLibResources(String libFile, String destDir) {
        Log.d(TAG, "Extract from " + libFile);
        ZipFile zipFile = null;
        try {
            zipFile = new ZipFile(libFile);
            for (String resource : XWALK_LIB_RESOURCES) {
                String entryDir = "";
                if (isNativeLibrary(resource)) {
                    if(Build.CPU_ABI.equalsIgnoreCase("armeabi")) {
                        // We build armeabi-v7a native lib for both armeabi & armeabi-v7a
                        entryDir = "lib" + File.separator + "armeabi-v7a" + File.separator;
                    } else {
                        entryDir = "lib" + File.separator + Build.CPU_ABI + File.separator;
                    }
                } else if (isAsset(resource)) {
                    entryDir = "assets" + File.separator;
                }
                Log.d(TAG, "unzip " + entryDir + resource);
                ZipEntry entry = zipFile.getEntry(entryDir + resource);
                saveStreamToFile(zipFile.getInputStream(entry), new File(destDir, resource));
            }
        } catch (IOException | NullPointerException e) {
            Log.d(TAG, e.getLocalizedMessage());
            return false;
        } finally {
            try {
                zipFile.close();
            } catch (IOException | NullPointerException e) {
            }
        }
        return true;
    }

    private boolean isNativeLibrary(String resource) {
        return resource.endsWith(".so");
    }

    private boolean isAsset(String resource) {
        return resource.endsWith(".dat") || resource.endsWith(".pak");
    }

    private void saveStreamToFile(InputStream input, File file) throws IOException {
        Log.d(TAG, "Save to " + file.getAbsolutePath());
        IOException outputException = null;

        try {
            // If the input stream is already closed, this method will throw an IOException so that
            // we don't create an unused output stream.
            input.available();

            FileOutputStream output = new FileOutputStream(file);

            try {
                byte[] buffer = new byte[STREAM_BUFFER_SIZE];
                for (int len = 0; (len = input.read(buffer)) >= 0;) {
                    output.write(buffer, 0, len);
                }
            } catch (IOException e) {
                outputException = e;
            }

            try {
                output.flush();
            } catch (IOException e) {
                if (outputException == null) outputException = e;
            }

            try {
                output.close();
            } catch (IOException e) {
                if (outputException == null) outputException = e;
            }
        } catch (IOException e) {
            throw e;
        } finally {
            try {
                input.close();
            } catch (IOException e) {
            }
        }

        if (outputException != null) {
            if (file.isFile()) file.delete();
            throw outputException;
        }
    }
}
