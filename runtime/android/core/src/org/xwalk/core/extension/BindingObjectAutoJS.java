// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extension;

import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

public class BindingObjectAutoJS extends BindingObject {

    public Object handleMessage(MessageInfo info) {
        Object result = null;
        ReflectionHelper extReflect = mInstanceHelper.getExtension().getReflection();
        ReflectionHelper mReflection =
                extReflect.getReflectionByBindingClass(this.getClass().getName());
        try {
            result = mReflection.handleMessage(info, this);
        } catch (Exception e) {
            Log.e("BindingObjectAutoJs", e.toString());
        }
        return result;
    }

    public JsContextInfo getJsContextInfo() {
        return new JsContextInfo(mInstanceHelper.getId(),
                                 mInstanceHelper.getExtension(),
                                 this.getClass(), mObjectId);
    }

    /* Helper method to invoke JavaScript callback.
     *
     * Following message will be sent to JavaScript side:
     * {
     *  cmd:"invokeCallback"
     *  // need to combine the cid and instanceId in the same feild
     *  callInfo: an object contains the callback information(cid, vid)
     *  key: String
     *  args: args
     * }
     */
    public static void invokeJsCallback(JsContextInfo mInfo, String callbackId, Object... args) {
        JSONArray jsArgs;
        Object[] arr = (Object[]) args;
        if (arr.length == 1 && arr[0] instanceof JSONArray) {
            jsArgs = (JSONArray)(arr[0]);
        } else {
            jsArgs = (JSONArray)(ReflectionHelper.toSerializableObject(args));
        }
        try {
            JSONObject msgOut = new JSONObject();
            msgOut.put("cmd", "invokeCallback");
            msgOut.put("callbackId", callbackId);
            msgOut.put("args", jsArgs);
            mInfo.postMessage(msgOut);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void invokeJsCallback(String callbackId, Object... args) {
        invokeJsCallback(getJsContextInfo(), callbackId, args);
    }

    public static void invokeJsCallback(JsContextInfo mInfo, byte[] buffer) {
        mInfo.postMessage(buffer);
    }
    // In this case, callbackId should be the first 4 bytes of the buffer.
    public void invokeJsCallback(byte[] buffer) {
        getJsContextInfo().postMessage(buffer);
    }

    /* Trigger JavaScript handlers in Java side.
     *
     * Following message will be sent to JavaScript side:
     * { cmd:"dispatchEvent"
     *   type: pointed in "supportedEvents" string array
     *   data: a JSON data will passed to js
     * }
     */
    public static void dispatchEvent(JsContextInfo mInfo, String type, Object event) {
        if (!mInfo.getTargetReflect().isEventSupported(type)) {
            Log.w(mInfo.getTag(), "Unsupport event in extension: " + type);
            return;
        }
        try {
            JSONObject msgOut = new JSONObject(); 
            msgOut.put("cmd", "dispatchEvent");
            msgOut.put("constructorName", mInfo.getConstructorName());
            msgOut.put("objectId", mInfo.getObjectId());
            msgOut.put("type", type);
            msgOut.put("event", ReflectionHelper.toSerializableObject(event));
            mInfo.postMessage(msgOut);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void dispatchEvent(String type, Object event) {
        dispatchEvent(getJsContextInfo(), type, event);
    }

    public static void sendEvent(JsContextInfo mInfo, String type, Object event) {
        mInfo.getExtensionClient().sendEvent(type, event);
    }

    public void sendEvent(String type, Object event) {
        sendEvent(getJsContextInfo(), type, event);
    }

    /* Notify the JavaScript side that some property is updated by Java side.
     *
     * Following message will be sent to JavaScript side:
     * { cmd:"updateProperty"
     *   name: the name of property need to be updated
     * }
     */
    public static void updateProperty(JsContextInfo mInfo, String pName) {
        ReflectionHelper targetReflect = mInfo.getTargetReflect();
        if (!targetReflect.hasProperty(pName)) {
            Log.w(mInfo.getTag(), "Unexposed property in extension: " + pName);
            return;
        }
        boolean isStatic = targetReflect.getMemberInfo(pName).isStatic;
        try {
            JSONObject msgOut = new JSONObject();
            msgOut.put("cmd", "updateProperty");
            msgOut.put("objectId", isStatic ? "0" : mInfo.getObjectId());
            msgOut.put("constructorName", mInfo.getConstructorName());
            msgOut.put("name", pName);
            mInfo.postMessage(msgOut);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void updateProperty(String pName) {
        updateProperty(getJsContextInfo(), pName);
    }
}
