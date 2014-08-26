// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;

public class AuxiliaryActivity extends Activity
{
    private String mExtra = "Auxiliary";

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        Intent intent = getIntent();
        final Class clazz = (Class)intent.getSerializableExtra("From");
        Intent newIntent = new Intent(AuxiliaryActivity.this, clazz);
        intent.putExtra("From", mExtra);
        setResult(RESULT_OK, intent);
        finish();
    }
}
