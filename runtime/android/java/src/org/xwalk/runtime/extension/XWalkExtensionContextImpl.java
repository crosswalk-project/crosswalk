// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension;

import android.app.Activity;
import android.content.Context;

import org.xwalk.runtime.XWalkRuntimeViewProvider;

/*
 * Internal implementation class which bridges the capabilities to external
 * users.
 */
public class XWalkExtensionContextImpl extends XWalkExtensionContext {
    private Context mContext;
    private Activity mActivity;
    private XWalkExtensionManager mManager;

    public XWalkExtensionContextImpl(Context context, Activity activity, XWalkExtensionManager manager) {
        mContext = context;
        mActivity = activity;
        mManager = manager;
    }

    @Override
    public Object registerExtension(XWalkExtension extension) {
        return mManager.registerExtension(extension);
    }

    @Override
    public void unregisterExtension(XWalkExtension extension) {
        mManager.unregisterExtension(extension);
    }

    @Override
    public void postMessage(XWalkExtension extension, int instanceID, String message) {
        mManager.postMessage(extension, instanceID, message);
    }

    @Override
    public Context getContext() {
        return mContext;
    }

    @Override
    public Activity getActivity() {
        return mActivity;
    }
}
