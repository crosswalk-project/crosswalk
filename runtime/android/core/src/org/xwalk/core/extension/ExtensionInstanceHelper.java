// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extension;

import org.json.JSONObject;

public class ExtensionInstanceHelper {
    private BindingObjectStore mStore;
    XWalkExternalExtension mExtension;
    MessageHandler mHandler;
    int mId;

    public ExtensionInstanceHelper(XWalkExternalExtension extension, int id) {
        mId = id;
        mExtension = extension;
        // Copy all the handlers registered to the extension object
        // to each instance helper.
        mHandler = new MessageHandler(mExtension.getMessageHandler());

        // Register annotated APIs.
        if (mExtension.isAutoJS()) {
            ReflectionHelper.registerHandlers(
                    mExtension.getReflection(), mHandler, mExtension);
        }
        mStore = new BindingObjectStore(mHandler, this);
    }

    public int getId() {
        return mId;
    }

    public XWalkExternalExtension getExtension() {
        return mExtension;
    }

    public BindingObject getBindingObject(String objectId) {
       return mStore.getBindingObject(objectId);
    }

    public boolean addBindingObject(String objectId, BindingObject obj) {
       return mStore.addBindingObject(objectId, obj);
    }

    public BindingObject removeBindingObject(String objectId) {
       return mStore.removeBindingObject(objectId);
    }

    public Object handleMessage(String message) {
        MessageInfo info = new MessageInfo(mExtension, mId, message);
        return mHandler.handleMessage(info);
    }

    public Object handleMessage(byte[] message) {
        MessageInfo info = new MessageInfo(mExtension, mId, message);
        return mHandler.handleMessage(info);
    }
}
