// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.client.embedded.shell;

import android.content.Context;
import android.os.Bundle;
import android.os.Looper;
import android.os.MessageQueue;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnFocusChangeListener;
import android.view.ViewGroup.LayoutParams;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.util.Log;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;

import org.chromium.base.library_loader.LibraryLoader;
import org.chromium.content.browser.TracingControllerAndroid;
import org.xwalk.app.XWalkRuntimeActivityBase;

public class XWalkRuntimeClientEmbeddedShellActivity extends XWalkRuntimeActivityBase {
    // TODO(yongsheng): Add one flag to hide the url bar.
    private static final String TAG = XWalkRuntimeClientEmbeddedShellActivity.class.getName();

    private EditText mUrlTextView;
    private TracingControllerAndroid mTracingController;

    TracingControllerAndroid getTracingController() {
        if (mTracingController == null) {
            mTracingController = new TracingControllerAndroid(this);
        }
        return mTracingController;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        registerTracingReceiverWhenIdle();
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        // Passdown the key-up event to runtime view.
        if (getRuntimeView() != null &&
                getRuntimeView().onKeyUp(keyCode, event)) {
            return true;
        }

        return super.onKeyUp(keyCode, event);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        unregisterTracingReceiver();
    }

    private void registerTracingReceiverWhenIdle() {
        // Delay tracing receiver registration until the main loop is idle.
        Looper.myQueue().addIdleHandler(new MessageQueue.IdleHandler() {
            @Override
            public boolean queueIdle() {
                // Will retry if the native library is not initialized yet.
                if (!LibraryLoader.isInitialized()) return true;
                try {
                    getTracingController().registerReceiver(XWalkRuntimeClientEmbeddedShellActivity.this);
                } catch (SecurityException e) {
                    Log.w(TAG, "failed to register tracing receiver: " + e.getMessage());
                }
                return false;
            }
        });
    }

    private void unregisterTracingReceiver() {
        try {
            getTracingController().unregisterReceiver(this);
        } catch (SecurityException e) {
            Log.w(TAG, "failed to unregister tracing receiver: " + e.getMessage());
        } catch (IllegalArgumentException e) {
            Log.w(TAG, "failed to unregister tracing receiver: " + e.getMessage());
        }
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
        if (getRuntimeView().get() != null) {
            setContentView(R.layout.testshell_activity);
            LinearLayout container = (LinearLayout) findViewById(R.id.content_container);
            container.addView(getRuntimeView().get(),
                              LayoutParams.MATCH_PARENT,
                              LayoutParams.MATCH_PARENT);
            initializeUrlField();
        }
    }
}
