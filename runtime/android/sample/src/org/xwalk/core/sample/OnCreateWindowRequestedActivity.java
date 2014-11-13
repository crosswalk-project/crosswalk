// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import android.app.Activity;
import android.os.Bundle;
import android.view.KeyEvent;
import android.widget.LinearLayout;
import android.webkit.ValueCallback;

import java.util.LinkedList;

import org.xwalk.core.XWalkNavigationHistory;
import org.xwalk.core.XWalkUIClient;
import org.xwalk.core.XWalkView;

public class OnCreateWindowRequestedActivity extends XWalkBaseActivity {
    private LinearLayout mParent;
    private LinkedList<XWalkView> mXWalkViewHistory = new LinkedList<XWalkView>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mParent = new LinearLayout(OnCreateWindowRequestedActivity.this);
        setContentView(mParent);

        mXWalkView = new XWalkView(OnCreateWindowRequestedActivity.this, 
                OnCreateWindowRequestedActivity.this);
        setClient(mXWalkView);

        mParent.addView(mXWalkView);
        mXWalkViewHistory.add(mXWalkView);

        mXWalkView.load("file:///android_asset/create_window_1.html", null);
    }

    private void setClient(XWalkView view) {
        view.setUIClient(new XWalkUIClient(view) {
            @Override
            public boolean onCreateWindowRequested(XWalkView view, InitiateBy initiator,
                    ValueCallback<XWalkView> callback) {
                XWalkView newView = new XWalkView(OnCreateWindowRequestedActivity.this, 
                        OnCreateWindowRequestedActivity.this);
                setClient(newView);

                mParent.removeView(mXWalkViewHistory.getLast());
                mParent.addView(newView);
                mXWalkViewHistory.add(newView);

                callback.onReceiveValue(newView);
                return true;
            }
        });
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (event.getAction() == KeyEvent.ACTION_DOWN && keyCode == KeyEvent.KEYCODE_BACK) {
            if (mXWalkViewHistory.getLast().getNavigationHistory().canGoBack()) {
                mXWalkViewHistory.getLast().getNavigationHistory().navigate(
                        XWalkNavigationHistory.Direction.BACKWARD, 1);
                return true;
            } else if (mXWalkViewHistory.size() > 1) {
                mParent.removeView(mXWalkViewHistory.removeLast());
                mParent.addView(mXWalkViewHistory.getLast());
                return true;
            }
        }
        return super.onKeyDown(keyCode, event);
    }
}
