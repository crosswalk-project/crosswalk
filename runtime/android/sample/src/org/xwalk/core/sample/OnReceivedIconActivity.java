// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import android.app.Activity;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.Message;
import android.widget.ImageView;
import android.widget.Toast;

import org.xwalk.core.XWalkUIClient;
import org.xwalk.core.XWalkView;

public class OnReceivedIconActivity extends XWalkBaseActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mXWalkView = new XWalkView(OnReceivedIconActivity.this, OnReceivedIconActivity.this);
        setContentView(mXWalkView);

        mXWalkView.setUIClient(new XWalkUIClient(mXWalkView) {
            @Override
            public void onIconAvailable(XWalkView view, String url, Message msg) {
                msg.sendToTarget();
            }

            @Override
            public void onReceivedIcon(XWalkView view, String url, Bitmap icon) {
                Toast toast = Toast.makeText(OnReceivedIconActivity.this, "Favicon", 
                        Toast.LENGTH_LONG);

                ImageView favicon = new ImageView(OnReceivedIconActivity.this);
                favicon.setImageBitmap(icon);

                toast.setView(favicon);
                toast.show();
            }
        });

        mXWalkView.load("file:///android_asset/favicon.html", null);
    }
}
