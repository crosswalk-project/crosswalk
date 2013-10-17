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
 * The abstract class to provide the common implementation for inherited classes.
 * Here is to use XWalkExtensionManager to manage extension system.
 */
public abstract class XWalkRuntimeViewProviderBase implements XWalkRuntimeViewProvider {
    private Context mContext;
    private Activity mActivity;
    private XWalkExtensionManager mExtensionManager;
    private XWalkExtensionContextImpl mExtensionContext;

    XWalkRuntimeViewProviderBase(Context context, Activity activity) {
        mContext = context;
        mActivity = activity;
        mExtensionManager = new XWalkExtensionManager(context, activity, this);
    }

    @Override
    public void init(Context context, Activity activity) {
        mExtensionManager.loadExtensions();
    }

    @Override
    public XWalkExtensionContext getExtensionContext() {
        return mExtensionManager.getExtensionContext();
    }

    @Override
    public void onCreate() {
    }

    @Override
    public void onResume() {
        mExtensionManager.onResume();
    }

    @Override
    public void onPause() {
        mExtensionManager.onPause();
    }

    @Override
    public void onDestroy() {
        mExtensionManager.onDestroy();
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        mExtensionManager.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    public void onMessage(XWalkExtension extension, String message) {
        extension.onMessage(message);
    }

    @Override
    public String onSyncMessage(XWalkExtension extension, String message) {
        return extension.onSyncMessage(message);
    }
}
