// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.client;

import android.app.DownloadManager;
import android.app.DownloadManager.Request;
import android.content.pm.PackageManager;
import android.content.Context;
import android.net.Uri;
import android.os.Environment;
import android.webkit.MimeTypeMap;
import android.webkit.URLUtil;
import android.widget.Toast;
import android.Manifest;

import org.xwalk.core.DownloadListener;
import org.xwalk.core.R;

public class XWalkDefaultDownloadListener implements DownloadListener {
    private static String DOWNLOAD_START_TOAST;
    private static String DOWNLOAD_NO_PERMISSION_TOAST;

    private Context mContext;

    public XWalkDefaultDownloadListener(Context context) {
        mContext = context;

        DOWNLOAD_START_TOAST = mContext.getString(R.string.download_start_toast);
        DOWNLOAD_NO_PERMISSION_TOAST =
                mContext.getString(R.string.download_no_permission_toast);
    }

    @Override
    public void onDownloadStart(String url, String userAgent,
            String contentDisposition, String mimetype, long contentLength) {
        String fileName = getFileName(url, contentDisposition, mimetype);

        // Check whether we have permission to write file to external storage.
        // We only start download request if we have permission.
        if (!checkWriteExternalPermission()) return;

        Request request = new Request(Uri.parse(url));
        request.setDestinationInExternalPublicDir(
                Environment.DIRECTORY_DOWNLOADS, fileName);
        getDownloadManager().enqueue(request);
        popupMessages(DOWNLOAD_START_TOAST + fileName);
    }

    private String getFileName(String url, String contentDisposition, String mimetype) {
        String fileName = URLUtil.guessFileName(url, contentDisposition, mimetype);
        int extensionIndex = fileName.lastIndexOf(".");
        String extension = null;
        if ((extensionIndex > 1) && (extensionIndex < fileName.length())) {
            extension = fileName.substring(extensionIndex + 1);
        }

        // If the file name does not have a file extension, append extension based on MIME type.
        if (extension == null) {
            extension = MimeTypeMap.getSingleton().getExtensionFromMimeType(mimetype);
            if (extension != null) fileName = fileName + "." + extension;
        }
        return fileName;
    }

    private DownloadManager getDownloadManager() {
        DownloadManager downloadManager =
                (DownloadManager) mContext.getSystemService(Context.DOWNLOAD_SERVICE);
        return downloadManager;
    }

    private boolean checkWriteExternalPermission() {
        int result = mContext.checkCallingOrSelfPermission(
                Manifest.permission.WRITE_EXTERNAL_STORAGE);
        if (result == PackageManager.PERMISSION_GRANTED) return true;

        popupMessages(DOWNLOAD_NO_PERMISSION_TOAST);
        return false;
    }

    private void popupMessages(String message) {
        Toast.makeText(mContext, message, Toast.LENGTH_SHORT).show();
    }
}
