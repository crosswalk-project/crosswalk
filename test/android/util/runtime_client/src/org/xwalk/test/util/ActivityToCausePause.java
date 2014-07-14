// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.test.util;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;

public class ActivityToCausePause extends Activity {
    private static ActivityToCausePause instance = null;

    public static void pauseActivity(Context activity) {
        Intent intent = new Intent(activity, ActivityToCausePause.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_BROUGHT_TO_FRONT);
        activity.startActivity(intent);
    }

    public static void resumeActivity() {
        if (instance != null) instance.finish();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        assert(instance == null);
        instance = this;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        instance = null;
    }
}
