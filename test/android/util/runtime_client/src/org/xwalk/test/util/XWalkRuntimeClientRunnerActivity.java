// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.test.util;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.widget.LinearLayout;

public class XWalkRuntimeClientRunnerActivity extends Activity
{
    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
    }

    public LinearLayout getLinearLayout() {
        return null;
    }

    public Intent getSecondIntent() {
        return null;
    }

    public  int getResultCode() {
        return -1;
    }

    public int getRequestCode() {
        return -1;
    }
}
