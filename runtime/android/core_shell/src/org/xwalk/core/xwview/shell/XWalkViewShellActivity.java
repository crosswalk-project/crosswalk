// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.shell;

import java.util.HashMap;

import android.app.ActionBar;
import android.app.ActionBar.Tab;
import android.app.FragmentTransaction;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.Context;
import android.content.IntentFilter;
import android.graphics.drawable.ClipDrawable;
import android.os.Bundle;
import android.os.Looper;
import android.os.MessageQueue;
import android.support.v4.app.FragmentActivity;
import android.support.v4.view.ViewPager;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnFocusChangeListener;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;

import org.chromium.base.BaseSwitches;
import org.chromium.base.CommandLine;
import org.chromium.base.library_loader.LibraryLoader;
import org.chromium.content.browser.TracingControllerAndroid;
import org.xwalk.core.XWalkNavigationHistory;
import org.xwalk.core.XWalkPreferences;
import org.xwalk.core.XWalkResourceClient;
import org.xwalk.core.XWalkUIClient;
import org.xwalk.core.XWalkView;

public class XWalkViewShellActivity extends FragmentActivity
        implements ActionBar.TabListener, XWalkViewSectionFragment.OnXWalkViewCreatedListener {
    public static final String COMMAND_LINE_FILE = "/data/local/tmp/xwview-shell-command-line";
    private static final String TAG = XWalkViewShellActivity.class.getName();
    public static final String COMMAND_LINE_ARGS_KEY = "commandLineArgs";
    private static final long COMPLETED_PROGRESS_TIMEOUT_MS = 200;
    private static final String ACTION_LAUNCH_URL = "org.xwalk.core.xwview.shell.launch";

    private LinearLayout mToolbar;
    private EditText mUrlTextView;
    private ImageButton mPrevButton;
    private ImageButton mNextButton;
    private ImageButton mStopButton;
    private ImageButton mReloadButton;
    private ClipDrawable mProgressDrawable;
    private XWalkView mActiveView;
    private TracingControllerAndroid mTracingController;
    private BroadcastReceiver mReceiver;
    private ActionBar mActionBar;
    private SectionsPagerAdapter mSectionsPagerAdapter;
    private ViewPager mViewPager;
    private HashMap<XWalkView, Integer> mProgressMap;

    private Runnable mClearProgressRunnable = new Runnable() {
        @Override
        public void run() {
            mProgressDrawable.setLevel(0);
        }
    };

    TracingControllerAndroid getTracingController() {
        if (mTracingController == null) {
            mTracingController = new TracingControllerAndroid(this);
        }
        return mTracingController;
    }

    private void registerTracingReceiverWhenIdle() {
        // Delay tracing receiver registration until the main loop is idle.
        Looper.myQueue().addIdleHandler(new MessageQueue.IdleHandler() {
            @Override
            public boolean queueIdle() {
                // Will retry if the native library is not initialized yet.
                if (!LibraryLoader.isInitialized()) return true;
                try {
                    getTracingController().registerReceiver(XWalkViewShellActivity.this);
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

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        registerTracingReceiverWhenIdle();

        if (!CommandLine.isInitialized()) {
            CommandLine.initFromFile(COMMAND_LINE_FILE);
            String[] commandLineParams = getCommandLineParamsFromIntent(getIntent());
            if (commandLineParams != null) {
                CommandLine.getInstance().appendSwitchesAndArguments(commandLineParams);
            }
        }

        waitForDebuggerIfNeeded();

        setContentView(R.layout.testshell_activity);

        mActionBar = getActionBar();
        mActionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);
        mSectionsPagerAdapter = new SectionsPagerAdapter(this, getSupportFragmentManager(), mActionBar);

        mViewPager = (ViewPager) findViewById(R.id.pager);
        mViewPager.setAdapter(mSectionsPagerAdapter);
        mViewPager.setOnPageChangeListener(new ViewPager.SimpleOnPageChangeListener() {
            @Override
            public void onPageSelected(int position) {
                mActionBar.setSelectedNavigationItem(position);
            }
        });

        mProgressMap = new HashMap<XWalkView, Integer>();
        // Add two tabs.
        mActionBar.addTab(
                mActionBar.newTab()
                        .setText(mSectionsPagerAdapter.getPageTitle(0))
                        .setTabListener(this));
        mActionBar.addTab(
                mActionBar.newTab()
                        .setText(mSectionsPagerAdapter.getPageTitle(1))
                        .setTabListener(this));

        mToolbar = (LinearLayout) findViewById(R.id.toolbar);
        mProgressDrawable = (ClipDrawable) findViewById(R.id.toolbar).getBackground();

        IntentFilter intentFilter = new IntentFilter(ACTION_LAUNCH_URL);
        mReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                Bundle bundle = intent.getExtras();
                if (bundle == null)
                    return;

                if (bundle.containsKey("url")) {
                    String extra = bundle.getString("url");
                    if (mActiveView != null)
                        mActiveView.load(sanitizeUrl(extra), null);
                }
            }
        };
        registerReceiver(mReceiver, intentFilter);
    }

    @Override
    public void onPause() {
        super.onPause();
        mSectionsPagerAdapter.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
        mSectionsPagerAdapter.onResume();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mReceiver);
        unregisterTracingReceiver();
        mSectionsPagerAdapter.onDestroy();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (mActiveView != null) mActiveView.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    public void onNewIntent(Intent intent) {
        if (mActiveView != null) {
            if (!mActiveView.onNewIntent(intent)) super.onNewIntent(intent);
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
        mUrlTextView.setOnEditorActionListener(new OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if ((actionId != EditorInfo.IME_ACTION_GO) && (event == null ||
                        event.getKeyCode() != KeyEvent.KEYCODE_ENTER ||
                        event.getAction() != KeyEvent.ACTION_DOWN)) {
                    return false;
                }

                if (mActiveView == null) return true;
                mActiveView.load(sanitizeUrl(mUrlTextView.getText().toString()), null);
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
                    if (mActiveView == null) return;
                    mUrlTextView.setText(mActiveView.getUrl());
                }
            }
        });
    }

    private void initializeButtons() {
        mPrevButton = (ImageButton) findViewById(R.id.prev);
        mPrevButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mActiveView != null &&
                        mActiveView.getNavigationHistory().canGoBack()) {
                    mActiveView.getNavigationHistory().navigate(
                            XWalkNavigationHistory.Direction.BACKWARD, 1);
                }
            }
        });

        mNextButton = (ImageButton) findViewById(R.id.next);
        mNextButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mActiveView != null &&
                        mActiveView.getNavigationHistory().canGoForward()) {
                    mActiveView.getNavigationHistory().navigate(
                            XWalkNavigationHistory.Direction.FORWARD, 1);
                }
            }
        });

        mStopButton = (ImageButton) findViewById(R.id.stop);
        mStopButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mActiveView != null) mActiveView.stopLoading();
            }
        });

        mReloadButton = (ImageButton) findViewById(R.id.reload);
        mReloadButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mActiveView != null) mActiveView.reload(XWalkView.RELOAD_NORMAL);
            }
        });
    }

    private void initializeXWalkViewClients(XWalkView xwalkView) {
        xwalkView.setResourceClient(new XWalkResourceClient(xwalkView) {
            @Override
            public void onProgressChanged(XWalkView view, int newProgress) {
                if (view != mActiveView) return;
                mToolbar.removeCallbacks(mClearProgressRunnable);

                mProgressDrawable.setLevel((int) (100.0 * newProgress));
                mProgressMap.put(view, (int) (100.0 * newProgress));
                if (newProgress == 100) {
                    mToolbar.postDelayed(mClearProgressRunnable, COMPLETED_PROGRESS_TIMEOUT_MS);
                    mProgressMap.put(view, 0);
                }
                mUrlTextView.setText(mActiveView.getUrl());
            }
        });

        xwalkView.setUIClient(new XWalkUIClient(xwalkView) {
            @Override
            public void onReceivedTitle(XWalkView view, String title) {
                mSectionsPagerAdapter.setPageTitle(view, title);
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
    public void onTabReselected(Tab tab, FragmentTransaction ft) {
        // Do nothing here currently, just make compiler happy.
    }

    @Override
    public void onTabSelected(Tab tab, FragmentTransaction ft) {
        mViewPager.setCurrentItem(tab.getPosition());
        android.support.v4.app.Fragment fragment = mSectionsPagerAdapter.getItem(tab.getPosition());
        if (fragment!= null && fragment instanceof XWalkViewSectionFragment) {
            mActiveView = ((XWalkViewSectionFragment)fragment).getXWalkView();
        } else {
            mActiveView = null;
        }
        if (mActiveView != null) {
            mUrlTextView.setText(mActiveView.getUrl());
            if (mProgressMap.containsKey(mActiveView)) {
                mProgressDrawable.setLevel(mProgressMap.get(mActiveView));
            } else {
                mProgressDrawable.setLevel(0);
            }
        }
    }

    @Override
    public void onTabUnselected(Tab tab, FragmentTransaction ft) {
        // Do nothing here currently, just make compiler happy.
    }

    @Override
    public void onXWalkViewCreated(XWalkView view) {
        if (mActiveView == null) {
            mActiveView = view;
            initializeUrlField();
            initializeButtons();
            mUrlTextView.setText("");
            mProgressDrawable.setLevel(0);
        }
        initializeXWalkViewClients(view);
        mProgressMap.put(view, 0);
        XWalkPreferences.setValue(XWalkPreferences.REMOTE_DEBUGGING, true);
    }
}
