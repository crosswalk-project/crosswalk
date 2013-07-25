// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime;

import android.content.Context;
import android.content.Intent;
import android.widget.FrameLayout;

import org.xwalk.core.XWalkView;

/**
 * The implementation class for XWalkRuntimeView. It calls the interfaces provided
 * by runtime core and customizes the behaviors here.
 * Note that it's a provisional solution. Will need a new refactoring.
 */
class XWalkRuntimeViewProvider {
    private Context mContext;
    private XWalkView mXwalkView;

    public XWalkRuntimeViewProvider(Context context, XWalkRuntimeView runtime) {
        mContext = context;

        // TODO(yongsheng): do customizations for XWalkView. There will
        // be many callback classes which are needed to be implemented.
        mXwalkView = new XWalkView(context);
        runtime.addView(mXwalkView,
                new FrameLayout.LayoutParams(
                        FrameLayout.LayoutParams.MATCH_PARENT,
                        FrameLayout.LayoutParams.MATCH_PARENT));
    }

    public void loadAppFromUrl(String url) {
        mXwalkView.loadUrl(url);
    }

    // TODO(yongsheng): Enable SysApps manifest support.
    public void loadAppFromManifest(String manifestUrl) {
    }

    // TODO(yongsheng): Implement these 4 methods once they are
    // supported by class XWalkView.
    public void onCreate() {
    }

    public void onResume() {
    }

    public void onPause() {
    }

    public void onDestroy() {
    }

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        // TODO(yongsheng): Implement it.
    }

    // TODO(yongsheng): Enable this once the remote debugging feature is supported.
    public String enableRemoteDebugging(String frontEndUrl, String socketName) {
        // return mXwalkView.enableRemoteDebugging(socketName);
        return "";
    }

    public void disableRemoteDebugging() {
        // mXwalkView.disableRemoteDebugging();
    }
}
