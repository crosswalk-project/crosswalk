// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app.runtime;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.view.View;
import android.webkit.ValueCallback;

import org.xwalk.app.runtime.extension.XWalkRuntimeExtensionLoader;
import org.xwalk.core.XWalkFileChooser;
import org.xwalk.core.XWalkPreferences;
import org.xwalk.core.XWalkUIClient;
import org.xwalk.core.XWalkView;

/**
 * The implementation class for runtime core. It calls the methods provided
 * by runtime core and customizes the behaviors for runtime.
 */
class XWalkCoreProviderImpl implements XWalkRuntimeViewProvider {
    private Context mContext;
    private XWalkView mXWalkView;
    private XWalkFileChooser mFileChooser;

    private class XWalkRuntimeUIClient extends XWalkUIClient {
        private Activity mActivity;

        public XWalkRuntimeUIClient() {
            super(mXWalkView);

            if (mContext instanceof Activity) {
                mActivity = (Activity) mContext;
            }
        }

        @Override
        public void openFileChooser(XWalkView view, ValueCallback<Uri> uploadFile,
                String acceptType, String capture) {
            if (mActivity == null) return;

            if (mFileChooser == null) {
                mFileChooser = new XWalkFileChooser(mActivity);
            }
            mFileChooser.showFileChooser(uploadFile, acceptType, capture);
        }
    }

    public XWalkCoreProviderImpl(Context context) {
        mContext = context;
    }

    @Override
    public View getView() {
        return mXWalkView;
    }

    @Override
    public void onCreate() {
        mXWalkView = new XWalkView(mContext);
        mXWalkView.setUIClient(new XWalkRuntimeUIClient());

        if (XWalkPreferences.getValue(XWalkPreferences.ENABLE_EXTENSIONS)) {
            XWalkRuntimeExtensionLoader.loadExtensions(mXWalkView, mContext);
        }
    }

    @Override
    public void onStart() {
        if (mXWalkView != null) mXWalkView.onShow();
    }

    @Override
    public void onResume() {
        if (mXWalkView != null) mXWalkView.resumeTimers();
    }

    @Override
    public void onPause() {
        if (mXWalkView != null) mXWalkView.pauseTimers();
    }

    @Override
    public void onStop() {
        if (mXWalkView != null) mXWalkView.onHide();
    }

    @Override
    public void onDestroy() {
        if (mXWalkView != null) mXWalkView.onDestroy();
    }

    @Override
    public boolean onNewIntent(Intent intent) {
        return mXWalkView != null && mXWalkView.onNewIntent(intent);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (mFileChooser != null) {
            mFileChooser.onActivityResult(requestCode, resultCode, data);
        }
    }

    @Override
    public String getVersion() {
        return mXWalkView.getXWalkVersion();
    }

    @Override
    public void loadAppFromUrl(String url) {
        mXWalkView.loadUrl(url);
    }

    @Override
    public void loadAppFromManifest(String manifestUrl) {
        mXWalkView.loadAppFromManifest(manifestUrl, null);
    }

    @Override
    public void setRemoteDebugging(boolean value) {
        XWalkPreferences.setValue(XWalkPreferences.REMOTE_DEBUGGING, value);
    }

    @Override
    public void setUseAnimatableView(boolean value) {
        XWalkPreferences.setValue(XWalkPreferences.ANIMATABLE_XWALK_VIEW, value);
    }

    // For instrumentation test.
    @Override
    public String getTitleForTest() {
        return mXWalkView.getTitle();
    }

    @Override
    public void setCallbackForTest(Object callback) {
        XWalkRuntimeTestHelper testHelper = new XWalkRuntimeTestHelper(mContext, mXWalkView);
        testHelper.setCallbackForTest(callback);
        mXWalkView.setUIClient(testHelper.getUIClient());
        mXWalkView.setResourceClient(testHelper.getResourceClient());
    }

    @Override
    public void loadDataForTest(String data, String mimeType, boolean isBase64Encoded) {
        mXWalkView.loadData(data, mimeType, isBase64Encoded ? "base64" : null);
    }
}
