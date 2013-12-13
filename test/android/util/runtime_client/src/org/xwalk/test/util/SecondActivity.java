// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.test.util;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.RelativeLayout;

public class SecondActivity extends Activity
{
    private Button mButton = null;
    private String mExtra = "Second";

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        RelativeLayout layout = new RelativeLayout(this);
        mButton = new Button(this);
        mButton.setText("Cbutton");
        layout.addView(mButton);
        setContentView(layout);

        Intent intent = getIntent();
        final Class className = (Class)intent.getSerializableExtra("From");
        try {
            mButton.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v)
                {
                    Intent intent = new Intent(SecondActivity.this, className);
                    intent.putExtra("From", mExtra);
                    setResult(RESULT_OK, intent);
                    finish();
                }
            });
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void clickButton() {
        mButton.performClick();
    }

    public String getExtraData() {
        return mExtra;
    }
}
