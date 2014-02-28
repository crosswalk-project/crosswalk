// Copyright (c) 2013 Intel Corporation. All rights reserved.
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

/**
 * The implementation class for runtime core. It calls the methods provided
 * by runtime core and customizes the behaviors for runtime.
 */
class XWalkCoreProviderImpl implements XWalkRuntimeViewProvider {
    private Context mContext;
    private Activity mActivity;
    private XWalkView mXwalkView;

    public XWalkCoreProviderImpl(Context context, Activity activity) {
        mContext = context;
        mActivity = activity;
        init(context, activity);
    }

    private void init(Context context, Activity activity) {
        // TODO(yongsheng): do customizations for XWalkView. There will
        // be many callback classes which are needed to be implemented.
        mXwalkView = new XWalkView(context, activity);
    }

    @Override
    public void loadAppFromUrl(String url) {
        mXwalkView.loadUrl(url);
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
        mXwalkView.loadAppFromManifest(path, manifest);
    }

    @Override
    public void onCreate() {
    }

    @Override
    public void onResume() {
        mXwalkView.onResume();
    }

    @Override
    public void onPause() {
        mXwalkView.onPause();
    }

    @Override
    public void onDestroy() {
        mXwalkView.onDestroy();
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        mXwalkView.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    public boolean onNewIntent(Intent intent) {
        return mXwalkView.onNewIntent(intent);
    }

    @Override
    public String enableRemoteDebugging(String frontEndUrl, String socketName) {
        // TODO(yongsheng): Enable two parameters once they're supported in XWalkView.
        return mXwalkView.enableRemoteDebugging();
    }

    @Override
    public void disableRemoteDebugging() {
        mXwalkView.disableRemoteDebugging();
    }

    @Override
    public String getVersion() {
        return mXwalkView.getVersion();
    }

    @Override
    public View getView() {
        return mXwalkView;
    }

    // For instrumentation test.
    @Override
    public String getTitleForTest() {
        return mXwalkView.getTitle();
    }

    @Override
    public void setCallbackForTest(Object callback) {
        XWalkClientForTest clientForTest = new XWalkClientForTest(mContext, mXwalkView);
        clientForTest.setCallbackForTest(callback);
        mXwalkView.setXWalkClient(clientForTest);

        XWalkWebChromeClientForTest webChromeClient =
                new XWalkWebChromeClientForTest(mContext, mXwalkView);
        webChromeClient.setCallbackForTest(callback);
        mXwalkView.setXWalkWebChromeClient(webChromeClient);
    }

    @Override
    public void loadDataForTest(String data, String mimeType, boolean isBase64Encoded) {
        mXwalkView.getXWalkViewContentForTest().getContentViewCoreForTest(
                ).loadUrl(LoadUrlParams.createLoadDataParams(
                        data, mimeType, isBase64Encoded));
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        return mXwalkView.onKeyUp(keyCode, event);
    }
}
