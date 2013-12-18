// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension.api.messaging;

import android.database.Cursor;
import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.util.ArrayList;
import java.util.Date;

import org.json.JSONException;
import org.json.JSONObject;
import org.xwalk.runtime.extension.api.messaging.MessagingSmsConstMaps;
import org.xwalk.runtime.extension.api.messaging.MessagingSmsConsts;

public class MessagingHelpers {
    private static String buildSqlClause(boolean hasAnd, String condition, String column) {
        String clause = hasAnd ? " AND " : ""; 
        clause += String.format(condition, column);
        return clause;
    }

    public static String convertJsDateString2Long(String date) {
        date = date.replace('T', ' ').replace('Z', ' ');
        long time = 0l;
        try {
            SimpleDateFormat sf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
            time = sf.parse(date).getTime();
        } catch (ParseException e) {
            e.printStackTrace();
        }

        return String.valueOf(time);
    }

    public static String convertDateLong2String(long time) {
        if (time <= 0l) {
            return "";
        }

        SimpleDateFormat sf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        return sf.format(new Date(time));
    }

    public static Object[] buildSqlFilterString(JSONObject filter) {
        String filterString = "";
        ArrayList<String> argsStringList = new ArrayList<String>();

        try {
            boolean hasAnd = false;
            
            if (filter.has("startDate")) {
                filterString += buildSqlClause(hasAnd, "%s >= ?", MessagingSmsConsts.DATE);
                argsStringList.add(convertJsDateString2Long(
                        filter.getString("startDate")));
                hasAnd = true;
            }

            if (filter.has("endDate")) {
                filterString += buildSqlClause(hasAnd, "%s <= ?", MessagingSmsConsts.DATE);
                argsStringList.add(convertJsDateString2Long(
                        filter.getString("endDate")));
                hasAnd = true;
            }

            if (filter.has("from")) {
                filterString += buildSqlClause(hasAnd, "%s = ?", MessagingSmsConsts.ADDRESS);
                argsStringList.add(filter.getString("from"));
                hasAnd = true;
            }
            
            String msgType = "sms";
            if (filter.has("type")) {
                msgType = filter.getString("type");
            }

            if (filter.has("state") && msgType.equals("sms")) {
                filterString += buildSqlClause(hasAnd, "%s = ?", MessagingSmsConsts.TYPE);
                Integer stateNum =
                    MessagingSmsConstMaps.smsStateDictS2I.get(filter.getString("state"));
                argsStringList.add(String.valueOf(stateNum));
                hasAnd = true;
            }

            if (filter.has("read")) {
                filterString += buildSqlClause(hasAnd, "%s = ?", MessagingSmsConsts.READ);
                argsStringList.add(filter.getBoolean("read") ? "1" : "0");
                hasAnd = true;
            }
        } catch (JSONException e) {
            e.printStackTrace();
            return null;
        }

        return new Object[]{filterString, argsStringList.toArray(new String[argsStringList.size()])};
    }

    public static String buildSqlFilterOptionString(JSONObject filterOption) {
        String filterOptionString = "";

        try {
            if (filterOption.has("sortBy")) {
                filterOptionString += " " +
                    MessagingSmsConstMaps.smsTableColumnDict.get(filterOption.getString("sortBy"));
            }

            if (filterOption.has("sortOrder")) {
                filterOptionString += " " + 
                    MessagingSmsConstMaps.sortOrderDict.get(filterOption.getString("sortOrder"));
            }

            if (filterOption.has("limit")) {
                filterOptionString += " LIMIT " + filterOption.getString("limit");
            }
        } catch (JSONException e) {
            e.printStackTrace();
            return "";
        }

        return filterOptionString;
    }

    public static JSONObject SmsMessageCursor2Json(Cursor c) {
        JSONObject jsonMsg = null;

        try {
            jsonMsg = new JSONObject();
            jsonMsg.put("messageID", c.getString(c.getColumnIndex(MessagingSmsConsts.ID)));
            jsonMsg.put("conversationID", c.getString(c.getColumnIndex(MessagingSmsConsts.THREAD_ID)));
            jsonMsg.put("type", "sms");
            jsonMsg.put("serviceID", "");
            jsonMsg.put("from", c.getString(c.getColumnIndex(MessagingSmsConsts.ADDRESS)));
            jsonMsg.put("timestamp", convertDateLong2String(
                    c.getLong(c.getColumnIndex(MessagingSmsConsts.DATE))));
            jsonMsg.put("read", c.getString(c.getColumnIndex(MessagingSmsConsts.READ)));
            jsonMsg.put("to", "");
            jsonMsg.put("body", c.getString(c.getColumnIndex(MessagingSmsConsts.BODY)));
            jsonMsg.put("state", MessagingSmsConstMaps.smsStateDictI2S.get(
                    c.getInt(c.getColumnIndex(MessagingSmsConsts.TYPE))));
            jsonMsg.put("deliveryStatus", MessagingSmsConstMaps.smsDiliveryStatusDictI2S.get(
                    c.getInt(c.getColumnIndex(MessagingSmsConsts.STATUS))));
            jsonMsg.put("deliveryTimestamp", "");
            jsonMsg.put("messageClass", "");
            return jsonMsg;
        } catch (JSONException e) {
            e.printStackTrace();
            return null;
        }
    }
}
