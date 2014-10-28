// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.client.shell;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnFocusChangeListener;
import android.view.ViewGroup.LayoutParams;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;

import org.xwalk.app.XWalkRuntimeActivityBase;
import org.xwalk.core.XWalkPreferences;

public class XWalkRuntimeClientShellActivity extends XWalkRuntimeActivityBase {
    // TODO(yongsheng): Add one flag to hide the url bar.
    private static final String TAG = XWalkRuntimeClientShellActivity.class.getName();

    private EditText mUrlTextView;
    private BroadcastReceiver mReceiver;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        IntentFilter intentFilter = new IntentFilter("org.xwalk.intent");
        intentFilter.addAction("android.intent.action.EXTERNAL_APPLICATIONS_AVAILABLE");
        intentFilter.addAction("android.intent.action.EXTERNAL_APPLICATIONS_UNAVAILABLE");
        mReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                Bundle bundle = intent.getExtras();
                if (bundle == null) return;

                XWalkPreferences.setValue(XWalkPreferences.REMOTE_DEBUGGING,
                        Boolean.parseBoolean(bundle.getString("remotedebugging", "false")));
            }
        };
        registerReceiver(mReceiver, intentFilter);
        super.onCreate(savedInstanceState);
    }

    @Override
    public void onDestroy() {
        unregisterReceiver(mReceiver);
        super.onDestroy();
    }

    private static String sanitizeUrl(String url) {
        if (url == null) return url;
        if (url.startsWith("www.") || url.indexOf(":") == -1) url = "http://" + url;
        return url;
    }

    private void initializeUrlField() {
        mUrlTextView = (EditText) findViewById(R.id.url);
        mUrlTextView.setOnEditorActionListener(new OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if ((actionId != EditorInfo.IME_ACTION_GO) && (event == null ||
                        event.getKeyCode() != KeyEvent.KEYCODE_ENTER ||
                        event.getAction() != KeyEvent.ACTION_DOWN)) {
                    return false;
                }

                getRuntimeView().loadAppFromUrl(sanitizeUrl(mUrlTextView.getText().toString()));
                mUrlTextView.clearFocus();
                setKeyboardVisibilityForUrl(false);
                return true;
            }
        });
        mUrlTextView.setOnFocusChangeListener(new OnFocusChangeListener() {
            @Override
            public void onFocusChange(View v, boolean hasFocus) {
                setKeyboardVisibilityForUrl(hasFocus);
                if (!hasFocus) {
                    // TODO(yongsheng): Fix this.
                    // mUrlTextView.setText(mRuntimeView.getUrl());
                }
            }
        });
    }

    private void setKeyboardVisibilityForUrl(boolean visible) {
        InputMethodManager imm = (InputMethodManager) getSystemService(
                Context.INPUT_METHOD_SERVICE);
        if (visible) {
            imm.showSoftInput(mUrlTextView, InputMethodManager.SHOW_IMPLICIT);
        } else {
            imm.hideSoftInputFromWindow(mUrlTextView.getWindowToken(), 0);
        }
    }

    @Override
    protected void didTryLoadRuntimeView(View runtimeView) {
        if (runtimeView != null) {
            setContentView(R.layout.testshell_activity);
            LinearLayout container = (LinearLayout) findViewById(R.id.content_container);
            container.addView(runtimeView,
                              LayoutParams.MATCH_PARENT,
                              LayoutParams.MATCH_PARENT);
            initializeUrlField();
        }
    }
}
