// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.extension.api.messaging;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.ContentResolver;  
import android.content.ContentValues;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;  
import android.net.Uri; 
import android.os.Bundle;
import android.telephony.SmsManager;
import android.telephony.SmsMessage;
import android.util.Log; 

import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.util.Date;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import org.xwalk.core.internal.extension.api.messaging.Messaging;
import org.xwalk.core.internal.extension.api.messaging.MessagingHelpers;
import org.xwalk.core.internal.extension.api.messaging.MessagingSmsConstMaps;
import org.xwalk.core.internal.extension.api.messaging.MessagingSmsConsts;

public class MessagingManager {
    private final static String TAG = "MessagingManager"; 
    private final Activity mMainActivity;
    private final Messaging mMessagingHandler;

    MessagingManager(Activity activity, Messaging messaging) {
        mMainActivity = activity;
        mMessagingHandler = messaging;
    }

    public void onMsgFindMessages(int instanceID, JSONObject jsonMsg) {
        queryMessage(instanceID, jsonMsg);
    }

    public void onMsgGetMessage(int instanceID, JSONObject jsonMsg) {
        queryMessage(instanceID, jsonMsg);
    }

    public void onMsgDeleteMessage(int instanceID, JSONObject jsonMsg) {
        operation(instanceID, jsonMsg);
    }

    public void onMsgDeleteConversation(int instanceID, JSONObject jsonMsg) {
        operation(instanceID, jsonMsg);
    }

    public void onMsgMarkMessageRead(int instanceID, JSONObject jsonMsg) {
        operation(instanceID, jsonMsg);
    }

    public void onMsgMarkConversationRead(int instanceID, JSONObject jsonMsg) {
        operation(instanceID, jsonMsg);
    }

    private Uri getUri(String type) {
        if (type.equals("mms")) {
            return Uri.parse("content://mms");
        } else {
            return Uri.parse("content://sms");
        }
    }

    private void queryMessage(int instanceID, JSONObject jsonMsg) {
        String promise_id = null, msgType = null, cmd = null, messageID = null;
        JSONObject filter = null, filterOption = null;
        
        try {
            promise_id = jsonMsg.getString("_promise_id");
            cmd = jsonMsg.getString("cmd");
            JSONObject eventBody = jsonMsg.getJSONObject("data");
            if (eventBody.has("messageID")) {
                messageID = eventBody.getString("messageID");
            }
            if (eventBody.has("filter")) {
                filter = eventBody.getJSONObject("filter");
            }
            if (eventBody.has("options")) {
                filterOption = eventBody.getJSONObject("options");
            }
            if (null != filter) {
                msgType = filter.getString("type");
            } else {
                msgType = eventBody.getString("type");
            }
        } catch (JSONException e) {
            e.printStackTrace();
            return;
        }

        if (!msgType.equals("sms") && !msgType.equals("mms")) {
            Log.e(TAG, "Invalidate message type: " + msgType);
            return;
        }

        ContentResolver cr = mMainActivity.getContentResolver();
        Uri contentUri = getUri(msgType);
        String sqlString = null;
        String[] sqlArgs = null;
        String sqlOption = null;

        if (cmd.equals("msg_findMessages")) {
            Object[] retValue = MessagingHelpers.buildSqlFilterString(filter);
            sqlString = (String)retValue[0];
            sqlArgs = (String[])retValue[1];
            sqlOption = MessagingHelpers.buildSqlFilterOptionString(filterOption);
        } else {
            sqlString = String.format("%s = ?", MessagingSmsConsts.ID);
            sqlArgs = new String[]{messageID};
        }

        Cursor cursor = cr.query(contentUri, null, sqlString, sqlArgs, sqlOption);

        JSONObject jsonMsgRet = null;
        JSONArray results = null;
        try {
            jsonMsgRet = new JSONObject();
            results = new JSONArray(); 
            jsonMsgRet.put("_promise_id", promise_id);
            jsonMsgRet.put("cmd", cmd + "_ret");
            JSONObject jsData = new JSONObject();
            jsonMsgRet.put("data", jsData);
            jsData.put("error", false);
            JSONObject jsBody = new JSONObject();
            jsData.put("body", jsBody);
            jsBody.put("results", results);
        } catch (JSONException e) {
            e.printStackTrace();
            return;
        }
        
        try {
            if (msgType.equals("mms")) {
                // TODO:(shawn) Pending on Android MMS related api get public. 
                // MMS is implemented in native messaging app, but they are not exposed as public APIs.
                // We ever tired to backport ~60 files with MMS feature supporting. Considering the pros and 
                // cons, we would rather break the MMS feature than doing the ugly backport.
            } else if (cursor.getCount() > 0) {
                while (cursor.moveToNext()) {
                    JSONObject jsonSmsObj = MessagingHelpers.SmsMessageCursor2Json(cursor);
                    if (null != jsonSmsObj) {
                        results.put(jsonSmsObj);
                    }
                }
            }
        } finally {
            cursor.close();
        }
        
        mMessagingHandler.postMessage(instanceID, jsonMsgRet.toString());
    }

    private void operation(int instanceID, JSONObject jsonMsg) {
        JSONObject eventBody = null;
        String promise_id = null, msgType = null, id = null, cmd = null;
        boolean isRead = false;

        try {
            promise_id = jsonMsg.getString("_promise_id");
            eventBody = jsonMsg.getJSONObject("data");
            if (eventBody.has("messageID")) {
                id = eventBody.getString("messageID");
            } else {
                id = eventBody.getString("conversationID");
            }
            cmd = jsonMsg.getString("cmd");
            if (eventBody.has("value")) {
                isRead = eventBody.getBoolean("value");
            }
            msgType = eventBody.getString("type");
        } catch (JSONException e) {
            e.printStackTrace();
            return;
        }

        String selString = null;
        if (eventBody.has("messageID")) {
            selString = String.format("%s = ?", MessagingSmsConsts.ID);
        } else {
            selString = String.format("%s = ?", MessagingSmsConsts.THREAD_ID);
        }

        String[] selArgs = new String[]{id};
        ContentResolver cr = mMainActivity.getContentResolver();
        Uri contentUri = getUri(msgType);

        if (cmd.equals("msg_deleteMessage") || cmd.equals("msg_deleteConversation")) {
            cr.delete(contentUri, selString, selArgs);
        } else if (cmd.equals("msg_markMessageRead") || cmd.equals("msg_markConversationRead")) {
            ContentValues values = new ContentValues();
            values.put("read", isRead ? "1" : "0"); 
            cr.update(contentUri, values, selString, selArgs);
        }

        JSONObject jsonMsgRet = null;
        try {
            jsonMsgRet = new JSONObject();
            jsonMsgRet.put("_promise_id", promise_id);
            JSONObject jsData = new JSONObject();
            jsonMsgRet.put("data", jsData);
            jsData.put("error", false);
            JSONObject jsBody = new JSONObject();
            jsData.put("body", jsBody);
            if (eventBody.has("messageID")) {
                jsBody.put("messageID", id);
            } else {
                jsBody.put("conversationID", id);
            }
            jsonMsgRet.put("cmd", cmd + "_ret");
        } catch (JSONException e) {
            e.printStackTrace();
            return;
        }

        mMessagingHandler.postMessage(instanceID, jsonMsgRet.toString());
    }
}
