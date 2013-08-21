// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.view.View;
import android.widget.FrameLayout;

import java.util.ArrayList;
import org.xwalk.runtime.XWalkRuntimeViewProvider;

/**
 * This internal class acts a manager to manage extensions.
 */
public class XWalkExtensionManager {
    // TODO(yongsheng): figure out how to support extension's configuration system.
    private Context mContext;
    private Activity mActivity;
    private XWalkRuntimeViewProvider mXwalkProvider;
    private XWalkExtensionContextImpl mExtensionContextImpl;

    private ArrayList<XWalkExtension> mExtensions;

    public XWalkExtensionManager(Context context, Activity activity, XWalkRuntimeViewProvider xwalkProvider) {
        mContext = context;
        mActivity = activity;
        mXwalkProvider = xwalkProvider;
        mExtensionContextImpl = new XWalkExtensionContextImpl(context, activity, this);
        mExtensions = new ArrayList<XWalkExtension>();
    }

    public XWalkExtensionContext getExtensionContext() {
        return mExtensionContextImpl;
    }

    public void postMessage(XWalkExtension extension, String message) {
        mXwalkProvider.postMessage(extension, message);
    }

    public Object registerExtension(XWalkExtension extension) {
        mExtensions.add(extension);
        return mXwalkProvider.onExtensionRegistered(extension);
    }

    public void unregisterExtensions() {
        for(XWalkExtension extension: mExtensions) {
            mXwalkProvider.onExtensionUnregistered(extension);
        }
        mExtensions.clear();
    }

    public void onResume() {
    }

    public void onPause() {
    }

    public void onDestroy() {
    }

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
    }
}
