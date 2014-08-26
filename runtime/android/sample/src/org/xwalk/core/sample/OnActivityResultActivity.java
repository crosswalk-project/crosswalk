// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import android.content.Intent;
import android.os.Bundle;
import android.widget.TextView;

public class OnActivityResultActivity extends XWalkBaseActivity {
    TextView mRequestCode;
    TextView mResultCode;
    TextView mData;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.result_layout);
        mRequestCode = (TextView) findViewById(R.id.result_text1);
        mResultCode = (TextView) findViewById(R.id.result_text2);
        mData = (TextView) findViewById(R.id.result_text3);

        Intent newIntent = new Intent(this, AuxiliaryActivity.class);
        newIntent.putExtra("From", this.getClass());
        this.startActivityForResult(newIntent, 2);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        String strRequest = "Request code should be '2': ";
        String strResultCode = "Result code should be '-1': ";
        String strExtra = "Extra data should be 'Auxiliary': ";
        Bundle extras = data.getExtras();

        strRequest += Integer.toString(requestCode);
        strResultCode += Integer.toString(resultCode);
        strExtra += extras.getString("From");
        mRequestCode.setText(strRequest);
        mResultCode.setText(strResultCode);
        mData.setText(strExtra);
    }
}
