// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import org.xwalk.core.sample.util.XWalkCommonFunctions;

import android.content.Intent;
import android.os.Bundle;
import android.widget.Button;
import android.widget.LinearLayout;

public class AuxiliaryActivityForNewIntent extends XWalkBaseActivity
{
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.result_layout);

        LinearLayout ll = (LinearLayout)findViewById(R.id.linearlayout);
        XWalkCommonFunctions commonFun = new XWalkCommonFunctions();
        commonFun.createComponentsAndAddView(ll, this, OnNewIntentActivity.class, "Second");
    }
}
