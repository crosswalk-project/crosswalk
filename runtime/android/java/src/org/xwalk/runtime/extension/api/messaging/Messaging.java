// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension.api.messaging;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;  
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;  
import android.net.Uri; 
import android.os.Bundle;
import android.telephony.SmsManager;
import android.telephony.SmsMessage;
import android.util.*; 
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import org.xwalk.runtime.extension.api.messaging.MessagingManager;
import org.xwalk.runtime.extension.api.messaging.MessagingSmsManager;
import org.xwalk.runtime.extension.XWalkExtension;
import org.xwalk.runtime.extension.XWalkExtensionContext;

interface Command {
    void runCommand(JSONObject jsonMsg);
}

public class Messaging extends XWalkExtension {
    public static final String NAME = "xwalk.experimental.messaging";
    public static final String JS_API_PATH = "jsapi/messaging_api.js";
    private static final String TAG = "Messaging";
    private static HashMap<String, Command> sMethodMap =
        new HashMap<String, Command>();

    private MessagingSmsManager smsManager;
    private MessagingManager messagingManager;

    private void initMethodMap() {
        try {
            sMethodMap.put("msg_smsSend", new Command() {
                public void runCommand(JSONObject jsonMsg) {
                    smsManager.onSmsSend(jsonMsg); 
                };
            });
            sMethodMap.put("msg_smsSegmentInfo", new Command() {
                public void runCommand(JSONObject jsonMsg) {
                    smsManager.onSmsSegmentInfo(jsonMsg);
                };
            });
            sMethodMap.put("msg_findMessages", new Command() {
                public void runCommand(JSONObject jsonMsg) {
                    messagingManager.onMsgFindMessages(jsonMsg);
                };
            });
            sMethodMap.put("msg_getMessage", new Command() {
                public void runCommand(JSONObject jsonMsg) {
                    messagingManager.onMsgGetMessage(jsonMsg);
                };
            });
            sMethodMap.put("msg_deleteMessage", new Command() {
                public void runCommand(JSONObject jsonMsg) {
                    messagingManager.onMsgDeleteMessage(jsonMsg);
                };
            });
            sMethodMap.put("msg_deleteConversation", new Command() {
                public void runCommand(JSONObject jsonMsg) {
                    messagingManager.onMsgDeleteConversation(jsonMsg);
                };
            });
            sMethodMap.put("msg_markMessageRead", new Command() {
                public void runCommand(JSONObject jsonMsg) {
                    messagingManager.onMsgMarkMessageRead(jsonMsg);
                };
            });
            sMethodMap.put("msg_markConversationRead", new Command() {
                public void runCommand(JSONObject jsonMsg) {
                    messagingManager.onMsgMarkConversationRead(jsonMsg);
                };
            });
        } catch(Exception e) {
            throw new RuntimeException(e);
        }
    }


    public Messaging(String jsApiContent, XWalkExtensionContext context) {
        super(NAME, jsApiContent, context);
        smsManager = new MessagingSmsManager(mExtensionContext.getActivity(), this);
        smsManager.Init(); //FIXME:(shawn) When onStart and OnStop are ready. This should be moved to onStart.
        messagingManager = new MessagingManager(mExtensionContext.getActivity(), this);

        initMethodMap();
    }

    @Override
    public void onDestroy() {
        smsManager.Uninit(); //FIXME:(shawn) When onStart and OnStop are ready. This should be moved to onStop.
    }

    @Override
    public void onMessage(int instanceID, String message) {
        if (message.isEmpty()) {
            return;
        }
        JSONObject jsonMsg = null;
        String commandString = null;

        try {
            jsonMsg = new JSONObject(message);
            commandString = jsonMsg.getString("cmd");
            
        } catch (JSONException e) {
            Log.e(TAG, e.toString());
            return;
        }

        try {
            Command command = sMethodMap.get(commandString);
            if (null != command) {
                command.runCommand(jsonMsg);
            }
        } catch(Exception e) {
          throw new RuntimeException(e);
        }
    }
}