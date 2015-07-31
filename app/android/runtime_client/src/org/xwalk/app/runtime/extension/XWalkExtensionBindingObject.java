// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app.runtime.extension;

import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

public class XWalkExtensionBindingObject {
    XWalkExtensionClient extensionClient;
    int myObjectId;
    int extInstanceId;

    public XWalkExtensionBindingObject(XWalkExtensionClient ext) {
        extensionClient = ext;
    }

    public void init(int instanceId, int objectId) {
        myObjectId = objectId;
        extInstanceId = instanceId;
    }

    /*
     * Called when this binding object is destoryed by JS.
     */
    public void onJsDestoryed() {}

    /*
     * Called when this object is binded with JS.
     */
    public void onJsBinded() {}

    public JsContextInfo getJsContextInfo() {
        return new JsContextInfo(extInstanceId, extensionClient, this.getClass(), myObjectId);
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
    public static void invokeJsCallback(JsContextInfo mInfo, JSONObject callInfo, String key, Object... args) {
        try {
            JSONObject jsCallInfo = new JSONObject();
            jsCallInfo.put("cid", callInfo.getInt("cid"));
            jsCallInfo.put("vid", callInfo.getInt("vid"));

            JSONObject msgOut = new JSONObject();
            msgOut.put("cmd", "invokeCallback");
            msgOut.put("constructorName", mInfo.getConstructorName());
            msgOut.put("objectId", mInfo.getObjectId());
            msgOut.put("callInfo", jsCallInfo);
            msgOut.put("key", key);
            msgOut.put("args", ReflectionHelper.objToJSON(args));
            mInfo.postMessage(msgOut);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void invokeJsCallback(JSONObject callInfo, String key, Object... args) {
        invokeJsCallback(getJsContextInfo(), callInfo, key, args);
    }

    /* Helper method to print information in JavaScript console,
     * mostly for debug purpose.
     *
     * Following message will be sent to JavaScript side:
     * { cmd:"error"
     *   level: "log", "info", "warn", "error", default is "error"
     *   msg: String
     * }
     */
    public static void logJs(JsContextInfo mInfo, String msg, String level) {
        try {
            JSONObject msgOut = new JSONObject(); 
            msgOut.put("cmd", "error");
            msgOut.put("level", level);
            msgOut.put("msg", msg);
            mInfo.postMessage(msgOut);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void logJs(String msg, String level) {
        logJs(getJsContextInfo(), msg, level);
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
            msgOut.put("event", ReflectionHelper.objToJSON(event));
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
            msgOut.put("objectId", isStatic ? 0 : mInfo.getObjectId());
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
