// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extension;

import android.util.Log;

public class BindingObject {
    private String TAG = "BindingObject";

    protected String mObjectId;
    protected ExtensionInstanceHelper mInstanceHelper;
    protected MessageHandler mHandler;

    public BindingObject() {
        mHandler = new MessageHandler();
    }

    public Object handleMessage(MessageInfo info) {
        return mHandler.handleMessage(info);
    }

    public void initBindingInfo(String objectId, ExtensionInstanceHelper instance) {
        mObjectId = objectId;
        mInstanceHelper = instance;
    }

    /*
     * Called when this binding object is destroyed by JS.
     */
    public void onJsDestroyed() {}

    /*
     * Called when this object is bound with JS.
     */
    public void onJsBound() {}

    public void onStart() {}
    public void onResume() {}
    public void onPause() {}
    public void onStop() {}
    public void onDestroy() {}
}
