// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extension.api.messaging;

import android.app.Activity;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;  
import android.net.Uri; 
import android.util.Log; 

import java.util.Date;
import java.util.HashMap;
import java.util.Map;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.xwalk.core.extension.api.messaging.MessagingManager;
import org.xwalk.core.extension.api.messaging.MessagingSmsManager;
import org.xwalk.core.extension.XWalkExtension;
import org.xwalk.core.extension.XWalkExtensionContext;

interface Command {
    void runCommand(int instanceID, JSONObject jsonMsg);
}

public class Messaging extends XWalkExtension {
    public static final String NAME = "xwalk.experimental.messaging";
    public static final String JS_API_PATH = "jsapi/messaging_api.js";
    private static HashMap<String, Command> sMethodMap = new HashMap<String, Command>();

    private MessagingSmsManager mSmsManager;
    private MessagingManager mMessagingManager;

    private void initMethodMap() {
        sMethodMap.put("msg_smsSend", new Command() {
            public void runCommand(int instanceID, JSONObject jsonMsg) {
                mSmsManager.onSmsSend(instanceID, jsonMsg); 
            };
        });
        sMethodMap.put("msg_smsClear", new Command() {
            public void runCommand(int instanceID, JSONObject jsonMsg) {
                mSmsManager.onSmsClear(instanceID, jsonMsg);
            };
        });
        sMethodMap.put("msg_smsSegmentInfo", new Command() {
            public void runCommand(int instanceID, JSONObject jsonMsg) {
                mSmsManager.onSmsSegmentInfo(instanceID, jsonMsg);
            };
        });
        sMethodMap.put("msg_findMessages", new Command() {
            public void runCommand(int instanceID, JSONObject jsonMsg) {
                mMessagingManager.onMsgFindMessages(instanceID, jsonMsg);
            };
        });
        sMethodMap.put("msg_getMessage", new Command() {
            public void runCommand(int instanceID, JSONObject jsonMsg) {
                mMessagingManager.onMsgGetMessage(instanceID, jsonMsg);
            };
        });
        sMethodMap.put("msg_deleteMessage", new Command() {
            public void runCommand(int instanceID, JSONObject jsonMsg) {
                mMessagingManager.onMsgDeleteMessage(instanceID, jsonMsg);
            };
        });
        sMethodMap.put("msg_deleteConversation", new Command() {
            public void runCommand(int instanceID, JSONObject jsonMsg) {
                mMessagingManager.onMsgDeleteConversation(instanceID, jsonMsg);
            };
        });
        sMethodMap.put("msg_markMessageRead", new Command() {
            public void runCommand(int instanceID, JSONObject jsonMsg) {
                mMessagingManager.onMsgMarkMessageRead(instanceID, jsonMsg);
            };
        });
        sMethodMap.put("msg_markConversationRead", new Command() {
            public void runCommand(int instanceID, JSONObject jsonMsg) {
                mMessagingManager.onMsgMarkConversationRead(instanceID, jsonMsg);
            };
        });
    }

    private String getCommandString(String message) {
        if (message.isEmpty()) {
            return "";
        }

        try {
            return new JSONObject(message).getString("cmd");
        } catch(Exception e) {
            e.printStackTrace();
            return "";
        }
    }

    public Messaging(String jsApiContent, XWalkExtensionContext context) {
        super(NAME, jsApiContent, context);
        mSmsManager = new MessagingSmsManager(mExtensionContext.getActivity(), this);
        mSmsManager.registerIntentFilters();
        //FIXME:(shawn) When onStart and OnStop are ready. This should be moved to onStart.
        mMessagingManager = new MessagingManager(mExtensionContext.getActivity(), this);

        initMethodMap();
    }

    @Override
    public void onDestroy() {
        mSmsManager.unregisterIntentFilters();
        //FIXME:(shawn) When onStart and OnStop are ready. This should be moved to onStop.
    }

    @Override
    public void onMessage(int instanceID, String message) {
        Command command = sMethodMap.get(getCommandString(message));

        if (null != command) {
            try {
                command.runCommand(instanceID, new JSONObject(message));
            } catch(Exception e) {
                e.printStackTrace();
                return;
            }
        }
    }

    @Override
    public String onSyncMessage(int instanceID, String message) {
        if (getCommandString(message).equals("msg_smsServiceId")) {
            return mSmsManager.getServiceIds();
        }
        return "";
    }
}
