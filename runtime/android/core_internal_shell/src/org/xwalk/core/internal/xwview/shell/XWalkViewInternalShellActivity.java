// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.xwview.shell;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.Context;
import android.content.IntentFilter;
import android.graphics.drawable.ClipDrawable;
import android.os.Bundle;
import android.util.Log;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnFocusChangeListener;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;

import org.chromium.base.BaseSwitches;
import org.chromium.base.CommandLine;
import org.xwalk.core.internal.XWalkNavigationHistoryInternal;
import org.xwalk.core.internal.XWalkPreferencesInternal;
import org.xwalk.core.internal.XWalkResourceClientInternal;
import org.xwalk.core.internal.XWalkViewInternal;

public class XWalkViewInternalShellActivity extends Activity {
    public static final String COMMAND_LINE_FILE = "/data/local/tmp/xwview-shell-command-line";
    private static final String TAG = XWalkViewInternalShellActivity.class.getName();
    public static final String COMMAND_LINE_ARGS_KEY = "commandLineArgs";
    private static final long COMPLETED_PROGRESS_TIMEOUT_MS = 200;
    private static final String ACTION_LAUNCH_URL = "org.xwalk.core.internal.xwview.shell.launch";

    private LinearLayout mToolbar;
    private EditText mUrlTextView;
    private ImageButton mPrevButton;
    private ImageButton mNextButton;
    private ImageButton mStopButton;
    private ImageButton mReloadButton;
    private ClipDrawable mProgressDrawable;
    private XWalkViewInternal mView;
    private BroadcastReceiver mReceiver;

    private Runnable mClearProgressRunnable = new Runnable() {
        @Override
        public void run() {
            mProgressDrawable.setLevel(0);
        }
    };

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
        mView = (XWalkViewInternal) findViewById(R.id.xwalkview);

        XWalkPreferencesInternal.setValue(XWalkPreferencesInternal.REMOTE_DEBUGGING, true);

        mToolbar = (LinearLayout) findViewById(R.id.toolbar);
        mProgressDrawable = (ClipDrawable) findViewById(R.id.toolbar).getBackground();
        mProgressDrawable.setLevel(0);

        initializeUrlField();
        initializeButtons();
        initializeXWalkViewInternalClients();

        IntentFilter intentFilter = new IntentFilter(ACTION_LAUNCH_URL);
        mReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                Bundle bundle = intent.getExtras();
                if (bundle == null) return;

                if (bundle.containsKey("url")) {
                    String extra = bundle.getString("url");
                    if (mView != null) mView.loadUrl(sanitizeUrl(extra));
                }
            }
        };
        registerReceiver(mReceiver, intentFilter);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mReceiver);
        mView.onDestroy();
    }

    @Override
    public void onNewIntent(Intent intent) {
        if (mView != null) {
            if (!mView.onNewIntent(intent)) super.onNewIntent(intent);
        }
    }

    private void waitForDebuggerIfNeeded() {
        if (CommandLine.getInstance().hasSwitch(BaseSwitches.WAIT_FOR_JAVA_DEBUGGER)) {
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
        mUrlTextView.setText("");
        mUrlTextView.setOnEditorActionListener(new OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if ((actionId != EditorInfo.IME_ACTION_GO) && (event == null ||
                        event.getKeyCode() != KeyEvent.KEYCODE_ENTER ||
                        event.getAction() != KeyEvent.ACTION_DOWN)) {
                    return false;
                }

                if (mView == null) return true;
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
                mStopButton.setVisibility(hasFocus ? View.GONE : View.VISIBLE);
                mReloadButton.setVisibility(hasFocus ? View.GONE : View.VISIBLE);
                if (!hasFocus) {
                    if (mView == null) return;
                    mUrlTextView.setText(mView.getUrl());
                }
            }
        });
    }

    private void initializeButtons() {
        mPrevButton = (ImageButton) findViewById(R.id.prev);
        mPrevButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mView != null &&
                        mView.getNavigationHistory().canGoBack()) {
                    mView.getNavigationHistory().navigate(
                            XWalkNavigationHistoryInternal.DirectionInternal.BACKWARD, 1);
                }
            }
        });

        mNextButton = (ImageButton) findViewById(R.id.next);
        mNextButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mView != null &&
                        mView.getNavigationHistory().canGoForward()) {
                    mView.getNavigationHistory().navigate(
                            XWalkNavigationHistoryInternal.DirectionInternal.FORWARD, 1);
                }
            }
        });

        mStopButton = (ImageButton) findViewById(R.id.stop);
        mStopButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mView != null) mView.stopLoading();
            }
        });

        mReloadButton = (ImageButton) findViewById(R.id.reload);
        mReloadButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mView != null) mView.reload(XWalkViewInternal.RELOAD_NORMAL);
            }
        });
    }

    private void initializeXWalkViewInternalClients() {
        mView.setResourceClient(new XWalkResourceClientInternal(mView) {
            @Override
            public void onProgressChanged(XWalkViewInternal view, int newProgress) {
                if (view != mView) return;
                mToolbar.removeCallbacks(mClearProgressRunnable);

                mProgressDrawable.setLevel((int) (100.0 * newProgress));
                if (newProgress == 100) {
                    mToolbar.postDelayed(mClearProgressRunnable, COMPLETED_PROGRESS_TIMEOUT_MS);
                }
                mUrlTextView.setText(mView.getUrl());
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
