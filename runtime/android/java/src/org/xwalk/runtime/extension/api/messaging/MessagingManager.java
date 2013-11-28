// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension.api.messaging;

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
import org.xwalk.runtime.extension.api.messaging.Messaging;
import org.xwalk.runtime.extension.api.messaging.MessagingHelpers;
import org.xwalk.runtime.extension.api.messaging.MessagingSmsConsts;
import org.xwalk.runtime.extension.api.messaging.MessagingSmsConstMaps;

public class MessagingManager {
    private static final String TAG = "MessagingManager";
    private Activity mMainActivity;
    private Messaging mMessagingHandler;

    MessagingManager(Activity activity, Messaging messaging) {
        mMainActivity = activity;
        mMessagingHandler = messaging;
    }

    public void onMsgFindMessages(JSONObject jsonMsg) {
        String _promise_id = null;
        String msgType = null;
        JSONObject eventBody = null;
        JSONObject filter = null;
        JSONObject filterOption = null;
        
        try {
            _promise_id = jsonMsg.getString("_promise_id");
            eventBody = jsonMsg.getJSONObject("data");
            filter = eventBody.getJSONObject("filter");
            filterOption = eventBody.getJSONObject("options");
            msgType = filter.getString("type");
        } catch (JSONException e) {
            e.printStackTrace();
            return;
        }

        
        Object[] retValue = MessagingHelpers.buildSqlFilterString(filter);
        String sqlFilter = (String)retValue[0];
        String[] sqlFilterArgs = (String[])retValue[1];
        String sqlfilterOption = 
            MessagingHelpers.buildSqlFilterOptionString(filterOption);

        ContentResolver cr = mMainActivity.getContentResolver();
        Uri contentUri = null;
        if (msgType.equals("sms")) {
            contentUri = Uri.parse("content://sms");
        }
        else if (msgType.equals("mms")) {
            contentUri = Uri.parse("content://mms");
        }
        else
        {
            contentUri = Uri.parse("content://sms");
        }
        
        Cursor cursor = 
            cr.query(contentUri, null, sqlFilter, sqlFilterArgs, sqlfilterOption);

        JSONObject jsonMsgRet = null;
        JSONArray results = null;
        try {
            jsonMsgRet = new JSONObject();
            results = new JSONArray(); 
            jsonMsgRet.put("_promise_id", _promise_id);
            jsonMsgRet.put("cmd", "msg_findMessages_ret");
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
        
        if (msgType.equals("mms")) {
            
        }
        else {
            if (cursor.getCount() > 0) {
                String count = Integer.toString(cursor.getCount());
                while (cursor.moveToNext()){
                    JSONObject jsonSmsObj = 
                        MessagingHelpers.SmsMessageCursor2Json(cursor);
                    if (null != jsonSmsObj) {
                        results.put(jsonSmsObj);
                    }
                }
            }
        }
        

        cursor.close();

        mMessagingHandler.broadcastMessage(jsonMsgRet.toString());
    }

    public void onMsgGetMessage(JSONObject jsonMsg) {
        String _promise_id = null;
        JSONObject eventBody = null;
        String msgType = null;
        String messageID = null;

        try {
            _promise_id = jsonMsg.getString("_promise_id");
            eventBody = jsonMsg.getJSONObject("data");
            messageID = eventBody.getString("messageID");
            msgType = eventBody.getString("type");
        } catch (JSONException e) {
            e.printStackTrace();
            return;
        }

        String selString = String.format("%s = ?", MessagingSmsConsts.ID);
        String[] selArgs = new String[]{messageID};

        ContentResolver cr = mMainActivity.getContentResolver();
        Uri contentUri = null;
        if (msgType.equals("sms")) {
            contentUri = Uri.parse("content://sms");
        }
        else if (msgType.equals("mms")) {
            contentUri = Uri.parse("content://mms");
        }
        else
        {
            contentUri = Uri.parse("content://sms");
        }

        Cursor cursor = cr.query(contentUri, null, selString, selArgs, null);

        JSONObject jsonMsgRet = null;
        JSONArray results = null;
        try {
            jsonMsgRet = new JSONObject();
            results = new JSONArray(); 
            jsonMsgRet.put("_promise_id", _promise_id);
            jsonMsgRet.put("cmd", "msg_getMessage_ret");
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
        
        if (msgType.equals("mms")) {
            //FIXME(shawn)
        }
        else {
            if (cursor.getCount() > 0) {
                String count = Integer.toString(cursor.getCount());
                while (cursor.moveToNext()){
                    JSONObject jsonSmsObj = 
                        MessagingHelpers.SmsMessageCursor2Json(cursor);
                    if (null != jsonSmsObj) {
                        results.put(jsonSmsObj);
                    }
                }
            }
        }
        

        cursor.close();

        mMessagingHandler.broadcastMessage(jsonMsgRet.toString());
    }

    public void onMsgDeleteMessage(JSONObject jsonMsg) {
        String _promise_id = null;
        JSONObject eventBody = null;
        String msgType = null;
        String messageID = null;

        try {
            _promise_id = jsonMsg.getString("_promise_id");
            eventBody = jsonMsg.getJSONObject("data");
            messageID = eventBody.getString("messageID");
            msgType = eventBody.getString("type");
        } catch (JSONException e) {
            e.printStackTrace();
            return;
        }

        String selString = String.format("%s = ?", MessagingSmsConsts.ID);
        String[] selArgs = new String[]{messageID};

        ContentResolver cr = mMainActivity.getContentResolver();
        Uri contentUri = null;

        if (msgType.equals("sms")) {
            contentUri = Uri.parse("content://sms");
        }
        else if (msgType.equals("mms")) {
            contentUri = Uri.parse("content://mms");
        }
        else
        {
            contentUri = Uri.parse("content://sms");
        }

        int deleteRows = cr.delete(contentUri, selString, selArgs);

        JSONObject jsonMsgRet = null;

        try {
            jsonMsgRet = new JSONObject();
            jsonMsgRet.put("_promise_id", _promise_id);
            jsonMsgRet.put("cmd", "msg_deleteMessage_ret");
            JSONObject jsData = new JSONObject();
            jsonMsgRet.put("data", jsData);
            jsData.put("error", false);
            JSONObject jsBody = new JSONObject();
            jsData.put("body", jsBody);
            jsBody.put("messageID", messageID);
        } catch (JSONException e) {
            e.printStackTrace();
            return;
        }

        mMessagingHandler.broadcastMessage(jsonMsgRet.toString());

    }

    public void onMsgDeleteConversation(JSONObject jsonMsg) {
        String _promise_id = null;
        JSONObject eventBody = null;
        String msgType = null;
        String conversationID = null;

        try {
            _promise_id = jsonMsg.getString("_promise_id");
            eventBody = jsonMsg.getJSONObject("data");
            conversationID = eventBody.getString("conversationID");
            msgType = eventBody.getString("type");
        } catch (JSONException e) {
            e.printStackTrace();
            return;
        }

        String selString = String.format("%s = ?", MessagingSmsConsts.THREAD_ID);
        String[] selArgs = new String[]{conversationID};

        ContentResolver cr = mMainActivity.getContentResolver();
        Uri contentUri = null;

        if (msgType.equals("sms")) {
            contentUri = Uri.parse("content://sms");
        }
        else if (msgType.equals("mms")) {
            contentUri = Uri.parse("content://mms");
        }
        else
        {
            contentUri = Uri.parse("content://sms");
        }

        int deleteRows = cr.delete(contentUri, selString, selArgs);

        JSONObject jsonMsgRet = null;

        try {
            jsonMsgRet = new JSONObject();
            jsonMsgRet.put("_promise_id", _promise_id);
            jsonMsgRet.put("cmd", "msg_deleteConversation_ret");
            JSONObject jsData = new JSONObject();
            jsonMsgRet.put("data", jsData);
            jsData.put("error", false);
            JSONObject jsBody = new JSONObject();
            jsData.put("body", jsBody);
            jsBody.put("conversationID", conversationID);
        } catch (JSONException e) {
            e.printStackTrace();
            return;
        }

        mMessagingHandler.broadcastMessage(jsonMsgRet.toString());

    }

    public void onMsgMarkMessageRead(JSONObject jsonMsg) {
        String _promise_id = null;
        JSONObject eventBody = null;
        String msgType = null;
        String messageID = null;
        boolean isRead = false;

        try {
            _promise_id = jsonMsg.getString("_promise_id");
            eventBody = jsonMsg.getJSONObject("data");
            messageID = eventBody.getString("messageID");
            msgType = eventBody.getString("type");
            isRead = eventBody.getBoolean("value");
        } catch (JSONException e) {
            e.printStackTrace();
            return;
        }

        String selString = String.format("%s = ?", MessagingSmsConsts.ID);
        String[] selArgs = new String[]{messageID};

        ContentResolver cr = mMainActivity.getContentResolver();
        Uri contentUri = null;

        if (msgType.equals("sms")) {
            contentUri = Uri.parse("content://sms");
        }
        else if (msgType.equals("mms")) {
            contentUri = Uri.parse("content://mms");
        }
        else
        {
            contentUri = Uri.parse("content://sms");
        }

        ContentValues values = new ContentValues();
        values.put("read", isRead?"1":"0"); 

        int updateRows = cr.update(contentUri, values, selString, selArgs);

        JSONObject jsonMsgRet = null;

        try {
            jsonMsgRet = new JSONObject();
            jsonMsgRet.put("_promise_id", _promise_id);
            jsonMsgRet.put("cmd", "msg_markMessageRead_ret");
            JSONObject jsData = new JSONObject();
            jsonMsgRet.put("data", jsData);
            jsData.put("error", false);
            JSONObject jsBody = new JSONObject();
            jsData.put("body", jsBody);
            jsBody.put("messageID", messageID);
        } catch (JSONException e) {
            e.printStackTrace();
            return;
        }

        mMessagingHandler.broadcastMessage(jsonMsgRet.toString());
    }

    public void onMsgMarkConversationRead(JSONObject jsonMsg) {
        String _promise_id = null;
        JSONObject eventBody = null;
        String msgType = null;
        String conversationID = null;
        boolean isRead = false;

        try {
            _promise_id = jsonMsg.getString("_promise_id");
            eventBody = jsonMsg.getJSONObject("data");
            conversationID = eventBody.getString("conversationID");
            msgType = eventBody.getString("type");
            isRead = eventBody.getBoolean("value");
        } catch (JSONException e) {
            e.printStackTrace();
            return;
        }

        String selString = String.format("%s = ?", MessagingSmsConsts.THREAD_ID);
        String[] selArgs = new String[]{conversationID};

        ContentResolver cr = mMainActivity.getContentResolver();
        Uri contentUri = null;

        if (msgType.equals("sms")) {
            contentUri = Uri.parse("content://sms");
        }
        else if (msgType.equals("mms")) {
            contentUri = Uri.parse("content://mms");
        }
        else
        {
            contentUri = Uri.parse("content://sms");
        }

        ContentValues values = new ContentValues();
        values.put("read", isRead?"1":"0"); 

        int updateRows = cr.update(contentUri, values, selString, selArgs);

        JSONObject jsonMsgRet = null;

        try {
            jsonMsgRet = new JSONObject();
            jsonMsgRet.put("_promise_id", _promise_id);
            jsonMsgRet.put("cmd", "msg_markConversationRead_ret");
            JSONObject jsData = new JSONObject();
            jsonMsgRet.put("data", jsData);
            jsData.put("error", false);
            JSONObject jsBody = new JSONObject();
            jsData.put("body", jsBody);
            jsBody.put("conversationID", conversationID);
        } catch (JSONException e) {
            e.printStackTrace();
            return;
        }

        mMessagingHandler.broadcastMessage(jsonMsgRet.toString());
    }
}