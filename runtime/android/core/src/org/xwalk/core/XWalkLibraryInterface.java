// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.net.Uri;

/**
 * Interface used by Crosswalk library
 */
interface XWalkLibraryInterface {
    /**
     * The Crosswalk library matches current application
     */
    public static final int STATUS_MATCH = 0;

    /**
     * The Crosswalk library is not found
     */
    public static final int STATUS_NOT_FOUND = 1;

    /**
     * The architecture of the Crosswalk library doesn't match current device
     */
    public static final int STATUS_ARCHITECTURE_MISMATCH = 2;

    /**
     * The integrity of the Crosswalk library can not be verified
     */
    public static final int STATUS_SIGNATURE_CHECK_ERROR = 3;

    /**
     * The version of the Crosswalk library is older than current application
     */
    public static final int STATUS_OLDER_VERSION = 4;

    /**
     * The version of the Crosswalk library is newer than current application
     */
    public static final int STATUS_NEWER_VERSION = 5;

    /**
     * Interface used to decompress native library
     */
    public interface DecompressionListener {
        /**
         * Runs on the UI thread to notify decompression is started
         */
        public void onDecompressionStarted();
        /**
         * Runs on the UI thread to notify decompression is cancelled
         */
        public void onDecompressionCancelled();
        /**
         * Runs on the UI thread to notify decompression is completed successfully
         */
        public void onDecompressionCompleted();
    }

    /**
     * Interface used to initialize the Crosswalk environment
     */
    public interface InitializationListener {
        /**
         * Runs on the UI thread to notify initialization is started
         */
        public void onInitializationStarted();
        /**
         * Runs on the UI thread to notify initialization is completed successfully
         */
        public void onInitializationCompleted();
    }

    /**
     * Interface used to download the Crosswalk library
     */
    public interface DownloadListener {
        /**
         * Runs on the UI thread to notify download is started
         */
        public void onDownloadStarted();
        /**
         * Runs on the UI thread to notify the download progress
         * @param percentage Shows the download progress in percentage
         */
        public void onDownloadUpdated(int percentage);
        /**
         * Runs on the UI thread to notify download is cancelled
         */
        public void onDownloadCancelled();
        /**
         * Runs on the UI thread to notify download is completed successfully
         * @param uri Uri where downloaded file is stored
         */
        public void onDownloadCompleted(Uri uri);
        /**
         * Runs on the UI thread to notify download failed
         *
         * @param status The download status that defined in android.app.DownloadManager.
         *               The value maybe STATUS_FAILED or STATUS_PAUSED
         * @param error The download error that defined in android.app.DownloadManager.
         *              This parameter only makes sense when the status is STATUS_FAILED
         */
        public void onDownloadFailed(int status, int error);
    }
}
