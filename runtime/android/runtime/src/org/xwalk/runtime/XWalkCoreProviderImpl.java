// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.view.KeyEvent;
import android.view.View;
import android.widget.FrameLayout;

import org.chromium.content.browser.LoadUrlParams;
import org.xwalk.core.XWalkView;
import org.xwalk.core.XWalkPreferences;

/**
 * The implementation class for runtime core. It calls the methods provided
 * by runtime core and customizes the behaviors for runtime.
 */
class XWalkCoreProviderImpl implements XWalkRuntimeViewProvider {
    private Context mContext;
    private Activity mActivity;
    private XWalkView mXWalkView;

    public XWalkCoreProviderImpl(Context context, Activity activity) {
        mContext = context;
        mActivity = activity;
        init(context, activity);
    }

    private void init(Context context, Activity activity) {
        // TODO(yongsheng): do customizations for XWalkView. There will
        // be many callback classes which are needed to be implemented.
        mXWalkView = new XWalkView(context, activity);
    }

    @Override
    public void loadAppFromUrl(String url) {
        mXWalkView.load(url, null);
    }

    @Override
    public void loadAppFromManifest(String manifestUrl) {
        XWalkManifestReader manifestReader = new XWalkManifestReader(mActivity);
        String manifest = manifestReader.read(manifestUrl);
        int position = manifestUrl.lastIndexOf("/");
        if (position == -1) {
            throw new RuntimeException("The URL of manifest file is invalid.");
        }

        String path = manifestUrl.substring(0, position + 1);
        mXWalkView.loadAppFromManifest(path, manifest);
    }

    @Override
    public void onCreate() {
    }

    @Override
    public void onResume() {
        mXWalkView.onShow();
    }

    @Override
    public void onPause() {
        mXWalkView.onHide();
    }

    @Override
    public void onDestroy() {
        mXWalkView.onDestroy();
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        mXWalkView.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    public boolean onNewIntent(Intent intent) {
        return mXWalkView.onNewIntent(intent);
    }

    @Override
    public void enableRemoteDebugging(String frontEndUrl, String socketName) {
        // TODO(yongsheng): Enable two parameters once they're supported in XWalkView.
        XWalkPreferences.setValue(XWalkPreferences.REMOTE_DEBUGGING, true);
    }

    @Override
    public void disableRemoteDebugging() {
        XWalkPreferences.setValue(XWalkPreferences.REMOTE_DEBUGGING, false);
    }

    @Override
    public String getVersion() {
        return mXWalkView.getXWalkVersion();
    }

    @Override
    public View getView() {
        return mXWalkView;
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
        mXWalkView.setXWalkClient(testHelper.getClient());
        mXWalkView.setXWalkWebChromeClient(testHelper.getWebChromeClient());
        mXWalkView.setResourceClient(testHelper.getResourceClient());
    }

    @Override
    public void loadDataForTest(String data, String mimeType, boolean isBase64Encoded) {
        mXWalkView.getXWalkViewContentForTest().getContentViewCoreForTest(
                ).loadUrl(LoadUrlParams.createLoadDataParams(
                        data, mimeType, isBase64Encoded));
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        return mXWalkView.onKeyUp(keyCode, event);
    }
}
