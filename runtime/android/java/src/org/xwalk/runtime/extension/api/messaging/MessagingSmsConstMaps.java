// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension.api.messaging;

import java.util.HashMap;
import org.xwalk.runtime.extension.api.messaging.MessagingSmsConsts;

public class MessagingSmsConstMaps {
    final public static HashMap<String, String> smsTableColumnDict = 
        new HashMap<String, String>();
    final public static HashMap<String, Integer> smsDeliveryStatusDictS2I = 
        new HashMap<String, Integer>();
    final public static HashMap<Integer, String> smsDiliveryStatusDictI2S = 
        new HashMap<Integer, String>();
    final public static HashMap<Integer, String> smsStateDictI2S = 
        new HashMap<Integer, String>();
    final public static HashMap<String, Integer> smsStateDictS2I = 
        new HashMap<String, Integer>();
    final public static HashMap<String, String> sortOrderDict = 
        new HashMap<String, String>();

    static {
        smsTableColumnDict.put("id", MessagingSmsConsts.ID);
        smsTableColumnDict.put("date", MessagingSmsConsts.DATE);
        smsTableColumnDict.put("from", MessagingSmsConsts.ADDRESS);
        smsTableColumnDict.put("state", MessagingSmsConsts.STATUS);
        smsTableColumnDict.put("error", MessagingSmsConsts.READ);
        smsDeliveryStatusDictS2I.put("success", -1);
        smsDeliveryStatusDictS2I.put("pending", 64);
        smsDeliveryStatusDictS2I.put("success", 0);
        smsDeliveryStatusDictS2I.put("error", 128);
        smsDiliveryStatusDictI2S.put(-1, "success");
        smsDiliveryStatusDictI2S.put(64, "pending");
        smsDiliveryStatusDictI2S.put(0, "success");
        smsDiliveryStatusDictI2S.put(128, "error");
        smsStateDictI2S.put(MessagingSmsConsts.MESSAGE_TYPE_INBOX, "received");
        smsStateDictI2S.put(MessagingSmsConsts.MESSAGE_TYPE_DRAFT, "draft");
        smsStateDictI2S.put(MessagingSmsConsts.MESSAGE_TYPE_OUTBOX, "sending");
        smsStateDictI2S.put(MessagingSmsConsts.MESSAGE_TYPE_QUEUED, "sending");
        smsStateDictI2S.put(MessagingSmsConsts.MESSAGE_TYPE_SENT, "sent");
        smsStateDictI2S.put(MessagingSmsConsts.MESSAGE_TYPE_FAILED, "failed");
        smsStateDictS2I.put("received", MessagingSmsConsts.MESSAGE_TYPE_INBOX);
        smsStateDictS2I.put("draft", MessagingSmsConsts.MESSAGE_TYPE_DRAFT);
        smsStateDictS2I.put("sending", MessagingSmsConsts.MESSAGE_TYPE_OUTBOX);
        smsStateDictS2I.put("sent", MessagingSmsConsts.MESSAGE_TYPE_SENT);
        smsStateDictS2I.put("failed", MessagingSmsConsts.MESSAGE_TYPE_FAILED);
        sortOrderDict.put("ascending", "ASC");
        sortOrderDict.put("descending", "DESC");
    }
}
