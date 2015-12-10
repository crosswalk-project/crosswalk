// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extension;

import android.util.Log;

import java.lang.Byte;
import java.lang.Integer;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.HashMap;
import java.util.Map;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

public class BindingObjectStore {
    private String TAG = "BindingObjectStore";
    private Map<String, BindingObject> mBindingObjects;
    private ExtensionInstanceHelper mInstance;

    public BindingObjectStore(MessageHandler handler,
            ExtensionInstanceHelper instance) {
        mBindingObjects = new HashMap<String, BindingObject>();
        mInstance = instance;
        handler.register("JSObjectCollected", "onJSObjectCollected", this);
        handler.register("postMessageToObject", "onPostMessageToObject", this);
        handler.register("postMessageToClass", "onPostMessageToClass", this);
    }

    public boolean addBindingObject(String objectId, BindingObject obj) {
        if (mBindingObjects.containsKey(objectId)) {
            Log.w(TAG, "Existing binding object:\n" + objectId);
            return false;
        }

        obj.initBindingInfo(objectId, mInstance);
        mBindingObjects.put(objectId, obj);
        obj.onJsBound();
        return true;
    }

    public BindingObject getBindingObject(String objectId) {
       return mBindingObjects.get(objectId);
    }

    public BindingObject removeBindingObject(String objectId) {
       BindingObject obj = mBindingObjects.remove(objectId);
       if (obj != null) obj.onJsDestroyed();

       return obj;
    }

    public void onJSObjectCollected(MessageInfo info) {
        removeBindingObject(info.getObjectId());
    }

    public Object onPostMessageToClass(MessageInfo info) {
        Object result = null;
        JSONArray args = info.getArgs();
        try {
            // args format:
            // [memberName, [constructorJsName, memberArgs]]
            MessageInfo newInfo = new MessageInfo(info);
            String memberName = args.getString(0);
            JSONArray originArgs = args.getJSONArray(1);
            String ctorName = originArgs.getString(0);
            JSONArray memberArgs = originArgs.getJSONArray(1);

            newInfo.setJsName(memberName);
            newInfo.setArgs(memberArgs);

            ReflectionHelper reflection =
                    info.getExtension().getTargetReflect(ctorName);
            result = reflection.handleMessage(newInfo, null);
        } catch (JSONException e) {
            Log.e(TAG, e.toString());
        } catch (Exception e) {
            Log.e(TAG, e.toString());
        }
        return result;
    }

    public Object onPostMessageToObject(MessageInfo info) {
        Object result = null;
        try {
            BindingObject obj = getBindingObject(info.getObjectId());

            MessageInfo newInfo = new MessageInfo(info);
            if (info.getArgs() != null) {
                JSONArray args = info.getArgs();
                String objectMethodName = args.getString(0);
                JSONArray objectMethodArgs = args.getJSONArray(1);
                newInfo.setJsName(objectMethodName);
                newInfo.setArgs(objectMethodArgs);
            } else {
                ByteBuffer args = info.getBinaryArgs();
                args.order(ByteOrder.LITTLE_ENDIAN);
                int byteOffset = args.position();
                int methodNameLen = args.getInt(byteOffset);
                byteOffset += Integer.SIZE / Byte.SIZE;
                int alignedMethodNameLen = methodNameLen + (4 - methodNameLen % 4);
                String objectMethodName = new String(args.array(), byteOffset, methodNameLen);
                byteOffset += alignedMethodNameLen;
                int len = args.array().length - byteOffset;
                ByteBuffer objectMethodArgs = ByteBuffer.wrap(args.array(), byteOffset, len);
                newInfo.setJsName(objectMethodName);
                newInfo.setBinaryArgs(objectMethodArgs);
            }
            if (obj != null) result = obj.handleMessage(newInfo);

        } catch (JSONException | IndexOutOfBoundsException | NullPointerException e) {
            Log.e(TAG, e.toString());
        }
        return result;
    }

    public void onStart() {
        for (Map.Entry<String, BindingObject> entry : mBindingObjects.entrySet()) {
            BindingObject obj = entry.getValue();
            obj.onStart();
        }
    }

    public void onResume() {
        for (Map.Entry<String, BindingObject> entry : mBindingObjects.entrySet()) {
            BindingObject obj = entry.getValue();
            obj.onResume();
        }
    }

    public void onPause() {
        for (Map.Entry<String, BindingObject> entry : mBindingObjects.entrySet()) {
            BindingObject obj = entry.getValue();
            obj.onPause();
        }
    }

    public void onStop() {
        for (Map.Entry<String, BindingObject> entry : mBindingObjects.entrySet()) {
            BindingObject obj = entry.getValue();
            obj.onStop();
        }
    }

    public void onDestroy() {
        for (Map.Entry<String, BindingObject> entry : mBindingObjects.entrySet()) {
            BindingObject obj = entry.getValue();
            obj.onDestroy();
        }
    }
}
