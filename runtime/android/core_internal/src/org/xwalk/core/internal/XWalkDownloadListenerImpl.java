// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.app.DownloadManager;
import android.app.DownloadManager.Request;
import android.content.pm.PackageManager;
import android.content.Context;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Environment;
import android.webkit.MimeTypeMap;
import android.webkit.URLUtil;
import android.widget.Toast;
import android.Manifest;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.io.OutputStream;

import org.xwalk.core.internal.AndroidProtocolHandler;
import org.xwalk.core.internal.R;
import org.xwalk.core.internal.XWalkDownloadListenerInternal;

class XWalkDownloadListenerImpl extends XWalkDownloadListenerInternal {
    private Context mContext;

    public XWalkDownloadListenerImpl(Context context) {
        super(context);
        mContext = context;
    }

    @Override
    public void onDownloadStart(String url, String userAgent,
            String contentDisposition, String mimetype, long contentLength) {
        String fileName = getFileName(url, contentDisposition, mimetype);

        // Check whether we have permission to write file to external storage.
        // We only start download request if we have permission.
        if (!checkWriteExternalPermission()) return;

        Uri src = Uri.parse(url);
        if (src.getScheme().equals("http") || src.getScheme().equals("https")) {
            Request request = new Request(Uri.parse(url));
            request.addRequestHeader("User-Agent", userAgent);
            request.setDestinationInExternalPublicDir(
                    Environment.DIRECTORY_DOWNLOADS, fileName);
            getDownloadManager().enqueue(request);
            popupMessages(mContext.getString(R.string.download_start_toast) + fileName);
        } else {
            new FileTransfer(url, fileName).execute();
        }
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

        popupMessages(mContext.getString(R.string.download_no_permission_toast));
        return false;
    }

    private void popupMessages(String message) {
        Toast.makeText(mContext, message, Toast.LENGTH_SHORT).show();
    }

    private class FileTransfer extends AsyncTask<Void, Void, String> {
        String url;
        String fileName;

        public FileTransfer(String url, String fileName) {
            this.url = url;
            this.fileName = fileName;
        }

        @Override
        protected String doInBackground(Void... params) {
            OutputStream dstStream = null;
            InputStream srcStream = null;
            File dir = Environment.getExternalStoragePublicDirectory(
                    Environment.DIRECTORY_DOWNLOADS);
            File dst = new File(dir, fileName);
            if (dst.exists()) {
                return "Existed";
            }
            try {
                dstStream = new FileOutputStream(dst);
                srcStream = AndroidProtocolHandler.open(mContext, url);
                if (dstStream != null && srcStream != null) {
                    streamTransfer(srcStream, dstStream);
                }
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                try {
                    if (srcStream != null) srcStream.close();
                    if (dstStream != null) dstStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                    return "Failed";
                }
            }
            return "Finished";
        }

        @Override
        protected void onPostExecute(String result) {
            if (result.equals("Failed")) {
                popupMessages(mContext.getString(R.string.download_failed_toast));
            } else if (result.equals("Existed")) {
                popupMessages(mContext.getString(R.string.download_already_exists_toast));
            } else if (result.equals("Finished")) {
                popupMessages(mContext.getString(R.string.download_finished_toast));
            }
        }

        private void streamTransfer(InputStream src, OutputStream dst) throws IOException {
            // Transfer bytes from src to dst.
            byte[] buf = new byte[1024];
            int length = 0;
            while ((length = src.read(buf)) > 0) {
                dst.write(buf, 0, length);
            }
        }
    }
}
