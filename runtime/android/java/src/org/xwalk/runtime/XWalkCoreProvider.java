// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.view.View;
import android.widget.FrameLayout;

import org.xwalk.core.XWalkView;

/**
 * The implementation class for XWalkCoreProvider. It calls the interfaces provided
 * by runtime core and customizes the behaviors here.
 */
class XWalkCoreProvider implements XWalkRuntimeViewProvider {
    private Context mContext;
    private XWalkView mXwalkView;

    public XWalkCoreProvider(Context context, Activity activity) {
        mContext = context;

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
        // TODO(yongsheng): Implement it.
    }

    @Override
    public void onCreate() {
        // TODO(yongsheng): Pass it to extensions.
    }

    @Override
    public void onResume() {
        // TODO(yongsheng): Pass it to extensions.
        mXwalkView.onResume();
    }

    @Override
    public void onPause() {
        // TODO(yongsheng): Pass it to extensions.
        mXwalkView.onPause();
    }

    @Override
    public void onDestroy() {
        // TODO(yongsheng): Pass it to extensions.
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        // TODO(yongsheng): Pass it to extensions.
        mXwalkView.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    public String enableRemoteDebugging(String frontEndUrl, String socketName) {
        // TODO(yongsheng): Enable this once the remote debugging feature is supported.
        // return mXwalkView.enableRemoteDebugging(socketName);
        return "";
    }

    @Override
    public void disableRemoteDebugging() {
        mXwalkView.disableRemoteDebugging();
    }

    @Override
    public View getView() {
        return mXwalkView;
    }
}
