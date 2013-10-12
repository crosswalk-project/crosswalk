// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.view.View;

import org.xwalk.runtime.extension.XWalkExtension;
import org.xwalk.runtime.extension.XWalkExtensionContext;
import org.xwalk.runtime.extension.XWalkExtensionContextImpl;
import org.xwalk.runtime.extension.XWalkExtensionManager;

/**
 * The abstract class to provide the bridge between XWalkRuntimeView and the
 * real implementation like runtime core.
 *
 * This class is also used by XWalkExtensionManager to get the capability to
 * build runtime extension system.
 */
public abstract class XWalkRuntimeViewProvider {
    private Context mContext;
    private Activity mActivity;
    private XWalkExtensionManager mExtensionManager;
    private XWalkExtensionContextImpl mExtensionContext;

    XWalkRuntimeViewProvider(Context context, Activity activity) {
        mContext = context;
        mActivity = activity;
        mExtensionManager = new XWalkExtensionManager(context, activity, this);
    }

    public void init(Context context, Activity activity) {
        mExtensionManager.loadExtensions();
    }

    public XWalkExtensionContext getExtensionContext() {
        return mExtensionManager.getExtensionContext();
    }

    public void onCreate() {

    }

    public void onResume() {
        mExtensionManager.onResume();
    }

    public void onPause() {
        mExtensionManager.onPause();
    }

    public void onDestroy() {
        mExtensionManager.onDestroy();
    }

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        mExtensionManager.onActivityResult(requestCode, resultCode, data);
    }

    public abstract String getVersion();
    public abstract View getView();
    public abstract void loadAppFromUrl(String url);
    public abstract void loadAppFromManifest(String manifestUrl);
    public abstract String enableRemoteDebugging(String frontEndUrl, String socketName);
    public abstract void disableRemoteDebugging();

    /*
     * Once runtime extension is created, notify the internal mechanism so that
     * it can create/do something it wants.
     */
    public abstract Object onExtensionRegistered(XWalkExtension extension);
    public abstract void onExtensionUnregistered(XWalkExtension extension);

    /**
     * Pass messages from native extension system to runtime extension system.
     * Might be overrided to do customizations here.
     */
    public void onMessage(XWalkExtension extension, String message) {
        extension.onMessage(message);
    }

    /**
     * Pass synchronized messages.
     */
    public String onSyncMessage(XWalkExtension extension, String message) {
        return extension.onSyncMessage(message);
    }

    /**
     * Pass message from runtime extension system to native and then to JavaScript.
     */
    public abstract void postMessage(XWalkExtension extension, String message);

    // For instrumentation test.
    public abstract String getTitleForTest();
    public abstract void setCallbackForTest(Object callback);
}
