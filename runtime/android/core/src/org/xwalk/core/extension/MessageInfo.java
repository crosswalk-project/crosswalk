// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extension;

import android.os.Build;
import android.util.Log;

import java.lang.Byte;
import java.lang.Integer;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import org.xwalk.core.extension.XWalkExternalExtension;

public class MessageInfo {
    private String TAG = "MessageInfo";
    private XWalkExternalExtension mExtension;
    private int mInstanceId;
    private String mJsName;
    private String mCallbackId;
    private String mObjectId;
    private String mCmd;
    private JSONArray mArgs;
    private ByteBuffer mBinaryArgs;

    private int AlignedWith4Bytes(int length) {
        return length + (4 - length % 4);
    }

    public MessageInfo(MessageInfo info) {
        this.mExtension = info.mExtension;
        this.mInstanceId = info.mInstanceId;
        this.mJsName = info.mJsName;
        this.mCallbackId = info.mCallbackId;
        this.mObjectId = info.mObjectId;
        this.mArgs = info.mArgs;
        this.mCmd = info.mCmd;
    }

    public MessageInfo(
            XWalkExternalExtension extension, int instanceId, String message) {
        mExtension = extension;
        mInstanceId = instanceId;
        if (message.trim().charAt(0) == '[') {
            // The message format of "common" JS module, an array contains:
            // [functionName, callbackId, objectId, methodName, args]
            try {
                mArgs = new JSONArray(message);

                mCmd = "invokeNative";
                mJsName = mArgs.getString(0);
                mCallbackId = mArgs.getString(1);
                mObjectId = mArgs.getString(2);

                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
                    mArgs.remove(0);
                    mArgs.remove(0);
                    mArgs.remove(0);
                }
            } catch (JSONException e) {
                Log.e(TAG, e.toString());
            }
        } else {
            /*
             * The message format of "jsStub" JS module, an object contains:
             * {
             *   type: "postMessageToObject"/"postMessageToClass"/"postMessageToExtension"
             *   cmd: invokeNative/newInstance/JSObjectCollected/getProperty
             *        /setProperty
             *   name: xxx
             *   args: []
             * }
             */
            try {
                JSONObject m = new JSONObject(message);
                String cmd = m.getString("cmd");
                int objectId = m.getInt("objectId");

                mCmd = cmd;
                mObjectId = Integer.toString(objectId);
                // CallbackId was in the right place of args.
                // So not use the callbackId in this case.
                mCallbackId = Integer.toString(0);
                String msgType = m.getString("type");
                mArgs = new JSONArray();

                if (msgType.equals(JsStubGenerator.MSG_TO_EXTENSION)) {
                    mArgs = m.getJSONArray("args");
                    mJsName = m.getString("name");
                    if (mCmd.equals("newInstance")) {
                        mObjectId = mArgs.getString(0);
                        mArgs = mArgs.getJSONArray(1);
                    }
                } else {
                    mJsName = msgType;
                    // Put the memberName to the args.
                    mArgs.put(0, m.getString("name"));
                    mArgs.put(1, m.getJSONArray("args"));
                }
            } catch (JSONException e) {
                Log.e(TAG, e.toString());
            }
        }
    }

    // Currently binary message only
    public MessageInfo(
            XWalkExternalExtension extension, int instanceId, byte[] message) {
        mExtension = extension;
        mInstanceId = instanceId;
        mCmd = "invokeNative";
        try {
            mArgs = null;
            ByteBuffer buf = ByteBuffer.wrap(message);
            if (buf.order() != ByteOrder.LITTLE_ENDIAN) {
                buf.order(ByteOrder.LITTLE_ENDIAN);
            }

            int byteOffset = buf.position();
            int byteCountOfInt = Integer.SIZE / Byte.SIZE;
            int funcNameLen = buf.getInt(byteOffset);
            int alignedFuncNameLen = AlignedWith4Bytes(funcNameLen);
            byteOffset += byteCountOfInt;
            mJsName = new String(message, byteOffset, funcNameLen);

            byteOffset += alignedFuncNameLen;
            mCallbackId = Integer.toString(buf.getInt(byteOffset));

            byteOffset += byteCountOfInt;
            int objectIdLen = buf.getInt(byteOffset);
            int alignedObjectIdLen = AlignedWith4Bytes(objectIdLen);
            byteOffset += byteCountOfInt;
            mObjectId = new String(message, byteOffset, objectIdLen);

            byteOffset += alignedObjectIdLen;
            int len = message.length - byteOffset;
            mBinaryArgs = ByteBuffer.wrap(message, byteOffset, len);
        } catch (IndexOutOfBoundsException e) {
            Log.e(TAG, e.toString());
        } catch (NullPointerException e) {
            Log.e(TAG, e.toString());
        }
    }

    public String getJsName() { return mJsName; }
    public void setJsName(String JsName) { mJsName = JsName; }

    public JSONArray getArgs() { return mArgs; }
    public void setArgs(JSONArray args) { mArgs = args; }

    public ByteBuffer getBinaryArgs() { return mBinaryArgs; }
    public void setBinaryArgs(ByteBuffer args) { mBinaryArgs = args; }

    public String getObjectId() { return mObjectId; }
    public void setObjectId(String objectId) { mObjectId = objectId; }

    public String getCallbackId() { return mCallbackId; }
    public void setCallbackId(String callbackId) { mCallbackId = callbackId; }

    public String getCmd() {return mCmd;}

    public void postResult(JSONArray args) {
        try {
            JSONArray result = new JSONArray();
            result.put(0, mCallbackId);
            for (int i = 0; i < args.length(); i++) {
                result.put(i + 1, args.get(i));
            }
            Log.w(TAG, "postResult: " + result.toString());
            mExtension.postMessage(mInstanceId, result.toString());
        } catch (JSONException e) {
            Log.e(TAG, e.toString());
        }
    }

    public XWalkExternalExtension getExtension() {
        return mExtension;
    }

    public int getInstanceId() {
        return mInstanceId;
    }

    public ExtensionInstanceHelper getInstanceHelper() {
        return mExtension.getInstanceHelper(mInstanceId);
    }

    public void postResult(byte[] buffer) {
        mExtension.postBinaryMessage(mInstanceId, buffer);
    }
}
