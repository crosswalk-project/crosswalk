// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension.api.messaging;

import android.database.Cursor;
import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.util.ArrayList;
import java.util.Date;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.xwalk.runtime.extension.api.messaging.MessagingSmsConsts;
import org.xwalk.runtime.extension.api.messaging.MessagingSmsConstMaps;

public class MessagingHelpers {

    public static long convertDateString2Long(String date) {

        try {
            SimpleDateFormat sf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
            return sf.parse(date).getTime();
        } catch (ParseException e) {
            e.printStackTrace();
        }

        return 0l;
    }

    public static String convertDateLong2String(long time) {

        if (time > 0l) {
            SimpleDateFormat sf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
            Date date = new Date(time);
            return sf.format(date);
        }

        return "";
    }

    public static Object[] buildSqlFilterString(JSONObject filter){
        
        String filterString = "";
        String[] filterArgs = null;
        ArrayList<String> argsStringList = new ArrayList<String>();
        String startDate = null;
        String endDate = null;

        try {
            boolean hasAnd = false;
            String msgType = "sms";
            if (filter.has("type")) {
                msgType = filter.getString("type");
            }

            if (filter.has("startDate")){
                if (hasAnd) {
                    filterString += " AND ";
                }
                filterString += String.format("%s >= ?",
                    MessagingSmsConsts.DATE);
                argsStringList.add(String.valueOf(convertDateString2Long(
                        filter.getString("startDate").replace('T', ' ').replace('Z', ' '))));
                hasAnd = true;
            }

            if (filter.has("endDate")) {
                 if (hasAnd) {
                    filterString += " AND ";
                }
                filterString += String.format("%s <= ?",
                    MessagingSmsConsts.DATE);
                argsStringList.add(String.valueOf(convertDateString2Long(
                        filter.getString("endDate").replace('T', ' ').replace('Z', ' '))));
                hasAnd = true;
            }

            if (filter.has("from")) {
                 if (hasAnd) {
                    filterString += " AND ";
                }
                filterString += String.format("%s = ?'",
                    MessagingSmsConsts.ADDRESS);
                argsStringList.add(filter.getString("from"));
                hasAnd = true;
            }

            if (filter.has("state") && msgType.equals("sms")) {
                 if (hasAnd) {
                    filterString += " AND ";
                }

                Integer stateNum = 
                    new Integer(MessagingSmsConstMaps.smsStateDictS2I.get(filter.getString("state")));
                filterString += String.format("%s = ?",
                    MessagingSmsConsts.TYPE);
                argsStringList.add(String.valueOf(stateNum));
                hasAnd = true;
            }

            if (filter.has("read")) {
                 if (hasAnd) {
                    filterString += " AND ";
                }
                filterString += String.format("%s = ?",
                    MessagingSmsConsts.READ);
                argsStringList.add(filter.getBoolean("read")?"1":"0");
                hasAnd = true;
            }
            filterArgs = new String[argsStringList.size()];
            filterArgs = argsStringList.toArray(filterArgs);

        } catch (JSONException e) {
            e.printStackTrace();
            return null;
        }

        return new Object[]{filterString, filterArgs};
    }

    public static String buildSqlFilterOptionString(JSONObject filterOption){
        String filterOptionString = "";

        try {
            if (filterOption.has("sortBy")){
                filterOptionString += " ";
                filterOptionString += 
                    MessagingSmsConstMaps.smsTableColumnDict.get(filterOption.getString("sortBy"));
            }

            if (filterOption.has("sortOrder")){
                filterOptionString += " ";
                filterOptionString += 
                    MessagingSmsConstMaps.sortOrderDict.get(filterOption.getString("sortOrder"));
            }

            if (filterOption.has("limit")){
                filterOptionString += " LIMIT ";
                filterOptionString += filterOption.getString("limit");
            }

        } catch (JSONException e) {
            e.printStackTrace();
            return "";
        }

        return filterOptionString;

    }

    public static JSONObject SmsMessageCursor2Json(Cursor cursor){

        JSONObject jsonMsg = null;

        try {
            jsonMsg = new JSONObject();
            jsonMsg.put("messageID", 
                cursor.getString(cursor.getColumnIndex(MessagingSmsConsts.ID)));
            jsonMsg.put("conversationID", 
                cursor.getString(cursor.getColumnIndex(MessagingSmsConsts.THREAD_ID)));
            jsonMsg.put("type", "sms");
            jsonMsg.put("serviceID", "");
            jsonMsg.put("from", 
                cursor.getString(cursor.getColumnIndex(MessagingSmsConsts.ADDRESS)));
            jsonMsg.put("timestamp", 
                convertDateLong2String(
                    cursor.getLong(
                        cursor.getColumnIndex(MessagingSmsConsts.DATE))));
            jsonMsg.put("read", 
                cursor.getString(cursor.getColumnIndex(MessagingSmsConsts.READ)));
            jsonMsg.put("to", "");
            jsonMsg.put("body", 
                cursor.getString(cursor.getColumnIndex(MessagingSmsConsts.BODY)));
            jsonMsg.put("state", 
                MessagingSmsConstMaps.smsStateDictI2S.get(
                    cursor.getInt(
                        cursor.getColumnIndex(
                            MessagingSmsConsts.TYPE))));
            jsonMsg.put("deliveryStatus", 
                MessagingSmsConstMaps.smsDiliveryStatusDictI2S.get(
                    cursor.getInt(
                        cursor.getColumnIndex(
                            MessagingSmsConsts.STATUS))));
            jsonMsg.put("deliveryTimestamp", "");
            jsonMsg.put("messageClass", "");
        } catch (JSONException e) {
            e.printStackTrace();
            return null;
        }

        return jsonMsg;
    }
}