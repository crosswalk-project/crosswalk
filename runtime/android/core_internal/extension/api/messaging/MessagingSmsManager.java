// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.extension.api.messaging;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;  
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.telephony.SmsManager;
import android.telephony.SmsMessage;
import android.telephony.TelephonyManager;
import android.net.Uri; 
import android.util.Log; 

import java.lang.ref.WeakReference;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.UUID;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.xwalk.core.internal.extension.api.messaging.Messaging;

public class MessagingSmsManager {
    private final static String TAG = "MessagingSmsManager";
    private final static String EXTRA_MSGID = "asyncCallId";
    private final static String EXTRA_MSGTEXT = "message";
    private final static String EXTRA_MSGTO = "to";
    private final static String EXTRA_MSGINSTANCEID = "instanceid";
    private final static String EXTRA_UUID= "UUID";
    private final static String DEFAULT_SERVICE_ID = "sim0";
    private final WeakReference<Activity> mActivity;
    private final Messaging mMessagingHandler;
    private BroadcastReceiver mSmsSentReceiver, mSmsDeliveredReceiver,
                              mSmsReceiveReceiver, mSmsServiceReceiver;
    private String mUUID;

    private abstract class MessagingReceiver extends BroadcastReceiver {
        protected Messaging mMessaging;

        public MessagingReceiver(Messaging messaging) {
            mMessaging = messaging;
        }
    }

    MessagingSmsManager(Activity activity, Messaging messaging) {
        mActivity = new WeakReference<Activity>(activity);
        mMessagingHandler = messaging;
        mUUID = UUID.randomUUID().toString();
    }

    private boolean checkService(String serviceID) {
        Activity activity = mActivity.get();
        if (activity == null) return false;
        TelephonyManager tm = 
            (TelephonyManager) activity.getSystemService(Context.TELEPHONY_SERVICE);
        return (TelephonyManager.SIM_STATE_READY == tm.getSimState());
    }

    public void onSmsSend(int instanceID, JSONObject jsonMsg) {
        if (!checkService(DEFAULT_SERVICE_ID)) {
            Log.e(TAG, "No Sim Card");
        }
        Activity activity = mActivity.get();
        if (activity == null) return;

        String asyncCallId = null;
        JSONObject eventBody = null;
        String phone = null;
        String smsMessage = null;
        try {
            asyncCallId = jsonMsg.getString("asyncCallId");
            eventBody = jsonMsg.getJSONObject("data");
            phone = eventBody.getString("phone");
            smsMessage = eventBody.getString("message");
        } catch (JSONException e) {
            e.printStackTrace();
            return;
        }
        
        SmsManager sms = SmsManager.getDefault();
        Intent intentSmsSent = new Intent("SMS_SENT");
        intentSmsSent.putExtra(EXTRA_MSGID, asyncCallId);
        intentSmsSent.putExtra(EXTRA_MSGTEXT, smsMessage);
        intentSmsSent.putExtra(EXTRA_MSGTO, phone);
        String instanceIDString = Integer.toString(instanceID);
        intentSmsSent.putExtra(EXTRA_MSGINSTANCEID, instanceIDString);
        intentSmsSent.putExtra(EXTRA_UUID, mUUID);
        int promiseIdInt = Integer.valueOf(asyncCallId);
        PendingIntent piSent = PendingIntent.getBroadcast(activity, 
                                                          promiseIdInt, 
                                                          intentSmsSent, 
                                                          PendingIntent.FLAG_ONE_SHOT);
        Intent intentSmsDelivered = new Intent("SMS_DELIVERED");
        intentSmsDelivered.putExtra(EXTRA_MSGID, asyncCallId);
        intentSmsDelivered.putExtra(EXTRA_MSGTEXT, smsMessage);
        intentSmsDelivered.putExtra(EXTRA_MSGINSTANCEID, instanceIDString);
        intentSmsDelivered.putExtra(EXTRA_UUID, mUUID);
        PendingIntent piDelivered = PendingIntent.getBroadcast(activity, 
                                                               -promiseIdInt, 
                                                               intentSmsDelivered,
                                                               PendingIntent.FLAG_ONE_SHOT);
        try {
            sms.sendTextMessage(phone, null, smsMessage, piSent, piDelivered);
        } catch (Exception e) {
            Log.e(TAG, "Failed to send SMS message.", e);
        }
    }

    public void onSmsClear(int instanceID, JSONObject jsonMsg) {
        Activity activity = mActivity.get();
        if (activity == null) return;

        String asyncCallId = null, cmd = null;
        JSONObject eventBody = null;
        String serviceID = null;
        try {
            asyncCallId = jsonMsg.getString("asyncCallId");
            cmd = jsonMsg.getString("cmd");
            eventBody = jsonMsg.getJSONObject("data");
            serviceID = eventBody.getString("serviceID");
        } catch (JSONException e) {
            e.printStackTrace();
            return;
        }

        ContentResolver cr = activity.getContentResolver();
        cr.delete(Uri.parse("content://sms"), null, null);

        JSONObject jsonMsgRet = null;
        try {
            jsonMsgRet = new JSONObject();
            jsonMsgRet.put("asyncCallId", asyncCallId);
            jsonMsgRet.put("cmd", cmd + "_ret");
            JSONObject jsData = new JSONObject();
            jsonMsgRet.put("data", jsData);
            jsData.put("error", false);
            JSONObject jsBody = new JSONObject();
            jsData.put("body", jsBody);
            jsBody.put("value", serviceID);
        } catch (JSONException e) {
            e.printStackTrace();
            return;
        }

        mMessagingHandler.postMessage(instanceID, jsonMsgRet.toString());
    }

    public void onSmsSegmentInfo(int instanceID, JSONObject jsonMsg) {
        String asyncCallId = null;
        JSONObject eventBody = null;
        String text = null;
        try {
            asyncCallId = jsonMsg.getString("asyncCallId");
            eventBody = jsonMsg.getJSONObject("data");
            text = eventBody.getString("text");

            if (null == text) {
                Log.e(TAG, "No \"text\" attribute.");
                return;
            }
        } catch (JSONException e) {
            e.printStackTrace();
            return;
        }
        
        SmsManager sms = SmsManager.getDefault();
        ArrayList<String> segs = sms.divideMessage(text);
        try {
            JSONObject jsonMsgRet = new JSONObject();
            jsonMsgRet.put("cmd", "msg_smsSegmentInfo_ret");
            jsonMsgRet.put("asyncCallId", asyncCallId);
            JSONObject jsData = new JSONObject();
            jsonMsgRet.put("data", jsData);
            jsData.put("error", false);
            JSONObject jsBody = new JSONObject();
            jsData.put("body", jsBody);
            jsBody.put("segments", segs.size());
            jsBody.put("charsPerSegment", segs.get(0).length());
            jsBody.put("charsAvailableInLastSegment", segs.get(segs.size() - 1).length());
            
            mMessagingHandler.postMessage(instanceID, jsonMsgRet.toString());
         } catch (JSONException e) {
            e.printStackTrace();
            return;
        }        
    }

    public void registerIntentFilters() {
        Activity activity = mActivity.get();
        if (activity == null) return;

        mSmsReceiveReceiver = new MessagingReceiver(mMessagingHandler) {
            @Override
            public void onReceive(Context context, Intent intent) {
                Bundle bundle = intent.getExtras();        
                
                if(null == bundle) {
                    return;
                }

                Object[] pdus = (Object[]) bundle.get("pdus");
                for (int i=0; i < pdus.length; ++i) {
                    try {
                        JSONObject jsonMsg = new JSONObject();
                        jsonMsg.put("cmd", "received");

                        SmsMessage msgs = SmsMessage.createFromPdu((byte[])pdus[i]);
                        
                        JSONObject jsData = new JSONObject();
                        jsonMsg.put("data", jsData);
                        JSONObject jsMsg = new JSONObject();
                        jsData.put("message", jsMsg);
                        jsMsg.put("messageID", "");
                        jsMsg.put("type", "sms");
                        jsMsg.put("serviceID", DEFAULT_SERVICE_ID);
                        jsMsg.put("from", msgs.getOriginatingAddress());
                        SimpleDateFormat sDateFormat = new SimpleDateFormat("yyyy-MM-dd hh:mm:ss");
                        jsMsg.put("timestamp", sDateFormat.format(new java.util.Date()));
                        jsMsg.put("body", msgs.getMessageBody().toString());
                        
                        mMessaging.broadcastMessage(jsonMsg.toString());
                     } catch (JSONException e) {
                        e.printStackTrace();
                        return;
                    }
                } 
            }
        };

        mSmsSentReceiver = new MessagingReceiver(mMessagingHandler) {
            @Override
            public void onReceive(Context content, Intent intent) {
                Activity activity = mActivity.get();
                if (activity == null) return;
                String uuid = intent.getStringExtra(EXTRA_UUID);
                if (null == uuid || !uuid.equals(mUUID)) return;

                boolean error = getResultCode() != Activity.RESULT_OK;
                String asyncCallId = intent.getStringExtra(EXTRA_MSGID);
                String smsMessage = intent.getStringExtra(EXTRA_MSGTEXT);
                String to = intent.getStringExtra(EXTRA_MSGTO);
                int instanceID = Integer.valueOf(intent.getStringExtra(EXTRA_MSGINSTANCEID));

                try {
                    JSONObject jsSentMsg = new JSONObject();
                    jsSentMsg.put("type", "sms");
                    jsSentMsg.put("from", "");
                    jsSentMsg.put("read", true);
                    jsSentMsg.put("to", to);
                    jsSentMsg.put("body", smsMessage);
                    jsSentMsg.put("messageClass", "class1");
                    jsSentMsg.put("state", error ? "failed" : "sending");
                    jsSentMsg.put("deliveryStatus", error ? "error" : "pending");

                    JSONObject jsonMsgPromise = new JSONObject();
                    jsonMsgPromise.put("asyncCallId", asyncCallId);
                    jsonMsgPromise.put("cmd", "msg_smsSend_ret");
                    JSONObject jsData = new JSONObject();
                    jsonMsgPromise.put("data", jsData);
                    jsData.put("error", error);
                    jsData.put("body", jsSentMsg);

                    mMessaging.postMessage(instanceID, jsonMsgPromise.toString());

                    JSONObject jsonMsgEvent = new JSONObject();
                    jsonMsgEvent.put("cmd", "sent");
                    jsonMsgEvent.put("data", jsSentMsg);

                    mMessaging.broadcastMessage(jsonMsgEvent.toString());
                } catch (JSONException e) {
                    e.printStackTrace();
                    return;
                }

                ContentValues values = new ContentValues();
                values.put("address", to);
                values.put("body", smsMessage);
                activity.getContentResolver().insert(Uri.parse("content://sms/sent"), values);
            }
        };

        mSmsDeliveredReceiver = new MessagingReceiver(mMessagingHandler) {
            @Override
            public void onReceive(Context content, Intent intent) {
                String uuid = intent.getStringExtra(EXTRA_UUID);
                if (null == uuid || !uuid.equals(mUUID)) return;

                boolean error = getResultCode() != Activity.RESULT_OK;
                String asyncCallId = intent.getStringExtra(EXTRA_MSGID);
                int instanceID = Integer.valueOf(intent.getStringExtra(EXTRA_MSGINSTANCEID));

                try {
                    JSONObject jsonMsg = new JSONObject();
                    jsonMsg.put("asyncCallId", asyncCallId);
                    jsonMsg.put("cmd", error ? "deliveryerror": "deliverysuccess");
                    JSONObject jsData = new JSONObject();
                    jsonMsg.put("data", jsData);
                    JSONObject jsEvent = new JSONObject();
                    jsData.put("event", jsEvent);
                    jsEvent.put("serviceID", DEFAULT_SERVICE_ID);
                    jsEvent.put("messageID", "");
                    jsEvent.put("recipients", "");
                    SimpleDateFormat sDateFormat = new SimpleDateFormat("yyyy-MM-dd hh:mm:ss");
                    jsEvent.put("deliveryTimestamps", sDateFormat.format(new java.util.Date()));
                    jsData.put("error", error);
                    mMessaging.postMessage(instanceID, jsonMsg.toString());
                } catch (JSONException e) {
                    e.printStackTrace();
                    return;
                }
            }
        };

        mSmsServiceReceiver = new MessagingReceiver(mMessagingHandler) {
            @Override
            public void onReceive(Context content, Intent intent) {
                try {
                    JSONObject jsonMsg = new JSONObject();
                    jsonMsg.put("cmd", 
                        checkService(DEFAULT_SERVICE_ID) ? "serviceadded": "serviceremoved");
                    JSONObject jsData = new JSONObject();
                    jsonMsg.put("data", jsData);
                    JSONObject jsEvent = new JSONObject();
                    jsData.put("event", jsEvent);
                    jsEvent.put("serviceID", DEFAULT_SERVICE_ID);
                    mMessaging.broadcastMessage(jsonMsg.toString());
                } catch (JSONException e) {
                    e.printStackTrace();
                    return;
                }
            }
        };

        activity.registerReceiver(
            mSmsReceiveReceiver, new IntentFilter("android.provider.Telephony.SMS_RECEIVED"));
        activity.registerReceiver(
            mSmsSentReceiver, new IntentFilter("SMS_SENT"));
        activity.registerReceiver(
            mSmsDeliveredReceiver,new IntentFilter("SMS_DELIVERED"));
        activity.registerReceiver(
            mSmsServiceReceiver,new IntentFilter("android.intent.action.SIM_STATE_CHANGED"));
    }

    public void unregisterIntentFilters() {
        Activity activity = mActivity.get();
        if (activity == null) return;

        activity.unregisterReceiver(mSmsReceiveReceiver);
        activity.unregisterReceiver(mSmsSentReceiver);
        activity.unregisterReceiver(mSmsDeliveredReceiver);
        activity.unregisterReceiver(mSmsServiceReceiver);
    }

    public String getServiceIds() {
        JSONArray serviceIds = new JSONArray();

        // FIXME:(shawn) Android doesn't has the concept of service ID, which means messages are
        // bundled to given SIM card. And official SDK does not support multiple SIM cards. 
        // So mock the default one to satisfy the spec.
        serviceIds.put(DEFAULT_SERVICE_ID);
        return serviceIds.toString();
    }
}
