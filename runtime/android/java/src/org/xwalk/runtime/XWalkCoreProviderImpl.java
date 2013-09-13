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
import org.xwalk.runtime.extension.XWalkExtension;

/**
 * The implementation class for runtime core. It calls the methods provided
 * by runtime core and customizes the behaviors for runtime.
 */
class XWalkCoreProviderImpl extends XWalkRuntimeViewProvider {
    private Context mContext;
    private XWalkView mXwalkView;

    public XWalkCoreProviderImpl(Context context, Activity activity) {
        super(context, activity);
        mContext = context;
        init(context, activity);
    }

    public void init(Context context, Activity activity) {
        // TODO(yongsheng): do customizations for XWalkView. There will
        // be many callback classes which are needed to be implemented.
        mXwalkView = new XWalkView(context, activity);
        super.init(context, activity);
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
        super.onCreate();
    }

    @Override
    public void onResume() {
        super.onResume();
        mXwalkView.onResume();
    }

    @Override
    public void onPause() {
        super.onPause();
        mXwalkView.onPause();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mXwalkView.onDestroy();
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        mXwalkView.onActivityResult(requestCode, resultCode, data);
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

    @Override
    public Object onExtensionRegistered(XWalkExtension extension) {
        // TODO(yongsheng): This object is supposed to register itself into native extension system.
        // If not, we'll need to register it.
        XWalkCoreExtensionBridge bridge = new XWalkCoreExtensionBridge(extension, this);
        return bridge;
    }

    @Override
    public void onExtensionUnregistered(XWalkExtension extension) {
        // TODO(yongsheng): Figure out how to do this.
    }

    @Override
    public void postMessage(XWalkExtension extension, String message) {
        XWalkCoreExtensionBridge bridge = (XWalkCoreExtensionBridge)extension.getRegisteredId();
        bridge.postMessage(message);
    }
}
