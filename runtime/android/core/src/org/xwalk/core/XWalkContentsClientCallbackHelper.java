// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

import org.chromium.content.browser.ContentViewCore;

/**
 * This class is responsible for calling certain client callbacks on the UI thread.
 *
 * Most callbacks do no go through here, but get forwarded to XWalkContentsClient directly. The
 * messages processed here may originate from the IO or UI thread.
 */
class XWalkContentsClientCallbackHelper {

    // TODO(boliu): Consider removing DownloadInfo and LoginRequestInfo by using native
    // MessageLoop to post directly to XWalkContent.

    private static class DownloadInfo {
        final String mUrl;
        final String mUserAgent;
        final String mContentDisposition;
        final String mMimeType;
        final long mContentLength;

        DownloadInfo(String url,
                     String userAgent,
                     String contentDisposition,
                     String mimeType,
                     long contentLength) {
            mUrl = url;
            mUserAgent = userAgent;
            mContentDisposition = contentDisposition;
            mMimeType = mimeType;
            mContentLength = contentLength;
        }
    }

    private static class LoginRequestInfo {
        final String mRealm;
        final String mAccount;
        final String mArgs;

        LoginRequestInfo(String realm, String account, String args) {
          mRealm = realm;
          mAccount = account;
          mArgs = args;
        }
    }

    private static class OnReceivedErrorInfo {
        final int mErrorCode;
        final String mDescription;
        final String mFailingUrl;

        OnReceivedErrorInfo(int errorCode, String description, String failingUrl) {
            mErrorCode = errorCode;
            mDescription = description;
            mFailingUrl = failingUrl;
        }
    }

    private final static int MSG_ON_LOAD_RESOURCE = 1;
    private final static int MSG_ON_PAGE_STARTED = 2;
    private final static int MSG_ON_DOWNLOAD_START = 3;
    private final static int MSG_ON_RECEIVED_LOGIN_REQUEST = 4;
    private final static int MSG_ON_RECEIVED_ERROR = 5;
    private final static int MSG_ON_RESOURCE_LOAD_STARTED = 6;

    private final XWalkContentsClient mContentsClient;

    private final Handler mHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(Message msg) {
            switch(msg.what) {
                case MSG_ON_LOAD_RESOURCE: {
                    final String url = (String) msg.obj;
                    mContentsClient.onLoadResource(url);
                    break;
                }
                case MSG_ON_PAGE_STARTED: {
                    final String url = (String) msg.obj;
                    mContentsClient.onPageStarted(url);
                    break;
                }
                case MSG_ON_DOWNLOAD_START: {
                    DownloadInfo info = (DownloadInfo) msg.obj;
                    mContentsClient.onDownloadStart(info.mUrl, info.mUserAgent,
                            info.mContentDisposition, info.mMimeType, info.mContentLength);
                    break;
                }
                case MSG_ON_RECEIVED_LOGIN_REQUEST: {
                    LoginRequestInfo info = (LoginRequestInfo) msg.obj;
                    mContentsClient.onReceivedLoginRequest(info.mRealm, info.mAccount, info.mArgs);
                    break;
                }
                case MSG_ON_RECEIVED_ERROR: {
                    OnReceivedErrorInfo info = (OnReceivedErrorInfo) msg.obj;
                    mContentsClient.onReceivedError(info.mErrorCode, info.mDescription,
                            info.mFailingUrl);
                    break;
                }
                case MSG_ON_RESOURCE_LOAD_STARTED: {
                    final String url = (String) msg.obj;
                    mContentsClient.onResourceLoadStarted(url);
                    break;
                }
                default:
                    throw new IllegalStateException(
                            "XWalkContentsClientCallbackHelper: unhandled message " + msg.what);
            }
        }
    };

    public XWalkContentsClientCallbackHelper(XWalkContentsClient contentsClient) {
        mContentsClient = contentsClient;
    }

    public void postOnLoadResource(String url) {
        mHandler.sendMessage(mHandler.obtainMessage(MSG_ON_LOAD_RESOURCE, url));
    }

    public void postOnPageStarted(String url) {
        mHandler.sendMessage(mHandler.obtainMessage(MSG_ON_PAGE_STARTED, url));
    }

    public void postOnDownloadStart(String url, String userAgent, String contentDisposition,
            String mimeType, long contentLength) {
        DownloadInfo info = new DownloadInfo(url, userAgent, contentDisposition, mimeType,
                contentLength);
        mHandler.sendMessage(mHandler.obtainMessage(MSG_ON_DOWNLOAD_START, info));
    }

    public void postOnReceivedLoginRequest(String realm, String account, String args) {
        LoginRequestInfo info = new LoginRequestInfo(realm, account, args);
        mHandler.sendMessage(mHandler.obtainMessage(MSG_ON_RECEIVED_LOGIN_REQUEST, info));
    }

    public void postOnReceivedError(int errorCode, String description, String failingUrl) {
        OnReceivedErrorInfo info = new OnReceivedErrorInfo(errorCode, description, failingUrl);
        mHandler.sendMessage(mHandler.obtainMessage(MSG_ON_RECEIVED_ERROR, info));
    }

    public void postOnResourceLoadStarted(String url) {
        mHandler.sendMessage(mHandler.obtainMessage(MSG_ON_RESOURCE_LOAD_STARTED, url));
    }
}
