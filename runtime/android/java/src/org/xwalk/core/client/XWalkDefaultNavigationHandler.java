// Copyright (c) 2013 Intel Corporation. All rights reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.client;

import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;

import org.chromium.components.navigation_interception.NavigationParams;

import org.xwalk.core.XWalkNavigationHandler;

public class XWalkDefaultNavigationHandler implements XWalkNavigationHandler {
    private static final String TAG = "XWalkDefaultNavigationHandler";
    private static final String PROTOCOL_WTAI_MC_PREFIX = "wtai://wp/mc;";

    private Context mContext;

    public XWalkDefaultNavigationHandler(Context context) {
        mContext = context;
    }

    @Override
    public boolean handleNavigation(NavigationParams params) {
        if (params.url.startsWith(PROTOCOL_WTAI_MC_PREFIX)) {
            return handlePhoneCall(params.url);
        }
        return false;
    }

    private boolean handlePhoneCall(String url) {
        String number = url.substring(PROTOCOL_WTAI_MC_PREFIX.length());
        String mcUrl= "tel:" + number;
        Intent intent = new Intent(Intent.ACTION_DIAL);
        intent.setData(Uri.parse(mcUrl));
        try {
            mContext.startActivity(intent);
        } catch (ActivityNotFoundException exception) {
            Log.w(TAG, "ACTION_DIAL: Activity not found.");
            return false;
        }
        return true;
    }
}
