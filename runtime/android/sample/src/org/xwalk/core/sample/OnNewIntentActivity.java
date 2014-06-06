// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import org.xwalk.core.sample.util.XWalkCommonFunctions;

import android.content.Intent;
import android.os.Bundle;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

public class OnNewIntentActivity extends XWalkBaseActivity {
    TextView mTextView1;
    TextView mTextView2;
    String mResult = "Test result: ";
    String mText = "onNewIntent works well.";
    String mStep = "Test step:\n1. Click the button(First) to next activity.\n" +
            "2. Click the button(Second) to return.\nYou can see \"" + mText +
                    "\" after" + mResult;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.result_layout);
        mTextView1 = (TextView) findViewById(R.id.result_text1);
        mTextView2 = (TextView) findViewById(R.id.result_text3);
        mTextView1.setText(mStep);
        mTextView2.setText(mResult);

        LinearLayout ll = (LinearLayout)findViewById(R.id.linearlayout);
        XWalkCommonFunctions commonFun = new XWalkCommonFunctions();
        commonFun.createComponentsAndAddView(ll, this, AuxiliaryActivityForNewIntent.class, "First");
    }

    @Override
    public void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        Bundle extras = intent.getExtras();
        String name = extras.getString("name");

        if (name.equals("crosswalk")) {
            String text = mResult + mText;
            mTextView2.setText(text);
        } else {
            mTextView2.setText(mResult);
        }
    }
}
