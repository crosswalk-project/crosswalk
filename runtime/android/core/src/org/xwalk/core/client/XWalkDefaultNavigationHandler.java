// Copyright (c) 2013 Intel Corporation. All rights reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.client;

import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;

import org.chromium.components.navigation_interception.NavigationParams;

import org.xwalk.core.XWalkNavigationHandler;

public class XWalkDefaultNavigationHandler implements XWalkNavigationHandler {
    private static final String TAG = "XWalkDefaultNavigationHandler";

    // WTAI prefix.
    private static final String PROTOCOL_WTAI_PREFIX = "wtai://";
    private static final String PROTOCOL_WTAI_MC_PREFIX = "wtai://wp/mc;";

    // Android action uri prefix.
    private static final String ACTION_TEL_PREFIX = "tel:";
    private static final String ACTION_SMS_PREFIX = "sms:";
    private static final String ACTION_MAIL_PREFIX = "mailto:";
    private static final String ACTION_GEO_PREFIX = "geo:";
    private static final String ACTION_MARKET_PREFIX = "market:";

    private Context mContext;

    public XWalkDefaultNavigationHandler(Context context) {
        mContext = context;
    }

    @Override
    public boolean handleNavigation(NavigationParams params) {
        final String url = params.url;
        Intent intent = null;
        if (url.startsWith(PROTOCOL_WTAI_PREFIX)) {
            intent = createIntentForWTAI(url);
        } else {
            intent = createIntentForActionUri(url);
        }
        return intent != null && startActivity(intent);
    }

    protected boolean startActivity(Intent intent) {
        try {
            if (!(mContext instanceof Activity)) {
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            }
            mContext.startActivity(intent);
        } catch (ActivityNotFoundException exception) {
            Log.w(TAG, "Activity not found for Intent:");
            Log.w(TAG, intent.toUri(0));
            return false;
        }
        return true;
    }

    private Intent createIntentForWTAI(String url) {
        Intent intent = null;
        if (url.startsWith(PROTOCOL_WTAI_MC_PREFIX)) {
            String number = url.substring(PROTOCOL_WTAI_MC_PREFIX.length());
            String mcUrl = ACTION_TEL_PREFIX + number;
            intent = new Intent(Intent.ACTION_DIAL);
            intent.setData(Uri.parse(mcUrl));
        }
        return intent;
    }

    private Intent createIntentForActionUri(String url) {
        Intent intent = null;
        if (url.startsWith(ACTION_TEL_PREFIX)) {
            // If dialing phone (tel:5551212).
            intent = new Intent(Intent.ACTION_DIAL);
            intent.setData(Uri.parse(url));
        } else if (url.startsWith(ACTION_GEO_PREFIX)) {
            // If displaying map (geo:0,0?q=address).
            intent = new Intent(Intent.ACTION_VIEW);
            intent.setData(Uri.parse(url));
        } else if (url.startsWith(ACTION_MAIL_PREFIX)) {
            // If sending email (mailto:abc@corp.com).
            intent = new Intent(Intent.ACTION_VIEW);
            intent.setData(Uri.parse(url));
        } else if (url.startsWith(ACTION_SMS_PREFIX)) {
            // If sms:5551212?body=This is the message.
            intent = new Intent(Intent.ACTION_VIEW);

            // Get address.
            String address = null;
            int parmIndex = url.indexOf('?');
            if (parmIndex == -1) {
                address = url.substring(4);
            } else {
                address = url.substring(4, parmIndex);

                // If body, then set sms body.
                Uri uri = Uri.parse(url);
                String query = uri.getQuery();
                if (query != null) {
                    if (query.startsWith("body=")) {
                        intent.putExtra("sms_body", query.substring(5));
                    }
                }
            }
            intent.setData(Uri.parse(ACTION_SMS_PREFIX + address));
            intent.putExtra("address", address);
            intent.setType("vnd.android-dir/mms-sms");
        } else if (url.startsWith(ACTION_MARKET_PREFIX)) {
            // If Android Market.
            intent = new Intent(Intent.ACTION_VIEW);
            intent.setData(Uri.parse(url));
        }
        return intent;
    }
}
