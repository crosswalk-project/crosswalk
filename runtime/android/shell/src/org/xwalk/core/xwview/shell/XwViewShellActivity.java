// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.shell;

import android.app.Activity;
import android.content.Intent;
import android.content.Context;
import android.graphics.drawable.ClipDrawable;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnFocusChangeListener;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;

import org.chromium.content.common.CommandLine;
import org.xwalk.core.XwView;
import org.xwalk.core.XwWebChromeClient;

public class XwViewShellActivity extends Activity {
    public static final String COMMAND_LINE_FILE = "/data/local/tmp/xwview-shell-command-line";
    private static final String TAG = XwViewShellActivity.class.getName();
    public static final String COMMAND_LINE_ARGS_KEY = "commandLineArgs";
    private static final long COMPLETED_PROGRESS_TIMEOUT_MS = 200;

    private LinearLayout mToolbar;
    private EditText mUrlTextView;
    private ImageButton mPrevButton;
    private ImageButton mNextButton;
    private XwView mView;
    private XwWebChromeClient.CustomViewCallback mCustomViewCallback;
    private FrameLayout mCustomViewContainer;
    private View mCustomView;


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (!CommandLine.isInitialized()) {
            CommandLine.initFromFile(COMMAND_LINE_FILE);
            String[] commandLineParams = getCommandLineParamsFromIntent(getIntent());
            if (commandLineParams != null) {
                CommandLine.getInstance().appendSwitchesAndArguments(commandLineParams);
            }
        }

        waitForDebuggerIfNeeded();

        setContentView(R.layout.testshell_activity);

        mView = (XwView) findViewById(R.id.content_container);
        mToolbar = (LinearLayout) findViewById(R.id.toolbar);
        mCustomViewContainer = (FrameLayout) findViewById(R.id.custom_view_container);

        initializeUrlField();
        initializeNavigationButtons();
        initializeXwViewClients();
    }

    private void waitForDebuggerIfNeeded() {
        if (CommandLine.getInstance().hasSwitch(CommandLine.WAIT_FOR_JAVA_DEBUGGER)) {
            Log.e(TAG, "Waiting for Java debugger to connect...");
            android.os.Debug.waitForDebugger();
            Log.e(TAG, "Java debugger connected. Resuming execution.");
        }
    }

    private static String[] getCommandLineParamsFromIntent(Intent intent) {
        return intent != null ? intent.getStringArrayExtra(COMMAND_LINE_ARGS_KEY) : null;
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
                        event.getKeyCode() != KeyEvent.ACTION_DOWN)) {
                    return false;
                }

                mView.loadUrl(sanitizeUrl(mUrlTextView.getText().toString()));
                mUrlTextView.clearFocus();
                setKeyboardVisibilityForUrl(false);
                return true;
            }
        });
        mUrlTextView.setOnFocusChangeListener(new OnFocusChangeListener() {
            @Override
            public void onFocusChange(View v, boolean hasFocus) {
                setKeyboardVisibilityForUrl(hasFocus);
                mNextButton.setVisibility(hasFocus ? View.GONE : View.VISIBLE);
                mPrevButton.setVisibility(hasFocus ? View.GONE : View.VISIBLE);
                if (!hasFocus) {
                    mUrlTextView.setText(mView.getUrl());
                }
            }
        });

    }

    private void initializeNavigationButtons() {
        mPrevButton = (ImageButton) findViewById(R.id.prev);
        mPrevButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mView.canGoBack()) mView.goBack();
            }
        });

        mNextButton = (ImageButton) findViewById(R.id.next);
        mNextButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mView.canGoForward()) mView.goForward();
            }
        });
    }

    private void initializeXwViewClients() {
        mView.setXwWebChromeClient(new XwWebChromeClient() {
            public void onProgressChanged(XwView view, int newProgress) {
                // TODO(yongsheng): Implement progress update.
            }

            @Override
            public void onShowCustomView(View view, CustomViewCallback callback) {
                if (mCustomView != null) callback.onCustomViewHidden();

                mCustomView = view;
                mView.setVisibility(View.GONE);
                mCustomViewContainer.setVisibility(View.VISIBLE);
                mCustomViewContainer.addView(view);
                mCustomViewCallback = callback;
            }

            @Override
            public void onHideCustomView() {
                super.onHideCustomView();
                if (mCustomView == null) return;

                mView.setVisibility(View.VISIBLE);
                mCustomViewContainer.setVisibility(View.GONE);
                mCustomView.setVisibility(View.GONE);
                mCustomViewContainer.removeView(mCustomView);
                mCustomViewCallback.onCustomViewHidden();

                mCustomView = null;
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
}
