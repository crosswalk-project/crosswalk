// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extension;

import org.json.JSONObject;

/*
 * This is a helper class to implement static members exported to constructors.
 */
public class JsContextInfo {
    private XWalkExternalExtension extensionClient;
    private String objectId;
    private Class<?> targetClass;
    private int extInstanceId;

    JsContextInfo(int instanceId, XWalkExternalExtension ext, Class<?> tClass, String objId) {
        extensionClient = ext;
        extInstanceId = instanceId;
        objectId = objId;
        targetClass = tClass;
    }

    public String getTag() {
        return "Extension-" + extensionClient.getExtensionName();
    }

    public ReflectionHelper getTargetReflect() {
        return extensionClient.getTargetReflect(targetClass.getSimpleName());
    }

    public String getObjectId() {
        return objectId;
    }

    public XWalkExternalExtension getExtensionClient() {
        return extensionClient;
    }

    public String getConstructorName() {
        return targetClass.getSimpleName();
    }

    public void postMessage(JSONObject msg) {
        extensionClient.postMessage(extInstanceId, msg.toString());
    }

    public void postMessage(byte[] buffer) {
        extensionClient.postBinaryMessage(extInstanceId, buffer);
    }
}
