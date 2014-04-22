// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import java.lang.Runnable;

import android.app.Activity;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.graphics.Rect;
import android.os.Bundle;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.LinearLayout;

import org.chromium.content.browser.ContentViewRenderView.FirstRenderedFrameListener;

/**
 * Provisionally set it as public due to the use of launch screen extension.
 * @hide
 */
public class XWalkLaunchScreenManager
        implements FirstRenderedFrameListener, DialogInterface.OnShowListener, PageLoadListener {
    // This string will be initialized before extension initialized,
    // and used by LaunchScreenExtension.
    private static String mIntentFilterStr;

    private XWalkView mXWalkView;
    private Activity mActivity;
    private Context mLibContext;
    private Dialog mLaunchScreenDialog;
    private boolean mPageLoadFinished;
    private ReadyWhenType mReadyWhen;
    private boolean mFirstFrameReceived;
    private BroadcastReceiver mLaunchScreenReadyWhenReceiver;
    private boolean mCustomHideLaunchScreen;

    private enum ReadyWhenType {
        FIRST_PAINT,
        USER_INTERACTIVE,
        COMPLETE,
        CUSTOM
    }

    public XWalkLaunchScreenManager(Context context, XWalkView xwView) {
        mXWalkView = xwView;
        mLibContext = context;
        mActivity = mXWalkView.getActivity();
        mIntentFilterStr = mActivity.getPackageName() + ".hideLaunchScreen";
    }

    public void displayLaunchScreen(String readyWhen) {
        if (mXWalkView == null) return;
        setReadyWhen(readyWhen);

        Runnable runnable = new Runnable() {
           public void run(){
                int resId = mActivity.getResources().getIdentifier(
                        "launchscreen", "drawable", mActivity.getPackageName());
                if (resId == 0) return;

                // Can not use the drawable directly in shared mode, need to decode it and create a new drawable.
                Bitmap bitmap = BitmapFactory.decodeResource(mActivity.getResources(), resId);
                Drawable finalDrawable = new BitmapDrawable(mActivity.getResources(), bitmap);

                mLaunchScreenDialog = new Dialog(mLibContext, android.R.style.Theme_Holo_Light_NoActionBar);
                if ((mActivity.getWindow().getAttributes().flags & WindowManager.LayoutParams.FLAG_FULLSCREEN) != 0) {
                    mLaunchScreenDialog.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, 
                            WindowManager.LayoutParams.FLAG_FULLSCREEN);
                }
                mLaunchScreenDialog.getWindow().setBackgroundDrawable(finalDrawable);
                mLaunchScreenDialog.setOnKeyListener(new Dialog.OnKeyListener() {
                    @Override
                    public boolean onKey(DialogInterface arg0, int keyCode,
                            KeyEvent event) {
                        if (keyCode == KeyEvent.KEYCODE_BACK) {
                            mLaunchScreenDialog.dismiss();
                            mActivity.onBackPressed();
                        }
                        return true;
                    }
                });
                mLaunchScreenDialog.setOnShowListener(XWalkLaunchScreenManager.this);
                mLaunchScreenDialog.show();
                if (mReadyWhen == ReadyWhenType.CUSTOM) registerBroadcastReceiver();
            }
        };
        mActivity.runOnUiThread(runnable);
    }

    @Override
    public void onFirstFrameReceived() {
        mFirstFrameReceived = true;
        hideLaunchScreenWhenReady();
    }

    @Override
    public void onShow(DialogInterface dialog) {
        mActivity.getWindow().setBackgroundDrawable(null);
        if (mFirstFrameReceived) hideLaunchScreenWhenReady();
    }

    @Override
    public void onPageFinished(String url) {
        mPageLoadFinished = true;
        hideLaunchScreenWhenReady();
    }

    public static String getHideLaunchScreenFilterStr() {
        return mIntentFilterStr;
    }

    private void registerBroadcastReceiver() {
        IntentFilter intentFilter = new IntentFilter(mIntentFilterStr);
        mLaunchScreenReadyWhenReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                mCustomHideLaunchScreen = true;
                hideLaunchScreenWhenReady();
            }
        };
        mActivity.registerReceiver(mLaunchScreenReadyWhenReceiver, intentFilter);
    }

    private void hideLaunchScreenWhenReady() {
        if (mLaunchScreenDialog == null || !mFirstFrameReceived) return;
        if (mReadyWhen == ReadyWhenType.FIRST_PAINT) {
            performHideLaunchScreen();
        } else if (mReadyWhen == ReadyWhenType.USER_INTERACTIVE) {
            // TODO: Need to listen js DOMContentLoaded event,
            // will be implemented in the next step.
            performHideLaunchScreen();
        } else if (mReadyWhen == ReadyWhenType.COMPLETE) {
            if (mPageLoadFinished) performHideLaunchScreen();
        } else if (mReadyWhen == ReadyWhenType.CUSTOM) {
            if (mCustomHideLaunchScreen) performHideLaunchScreen();
        }
    }

    private void performHideLaunchScreen() {
        mLaunchScreenDialog.dismiss();
        mLaunchScreenDialog = null;
        if (mReadyWhen == ReadyWhenType.CUSTOM) {
            mActivity.unregisterReceiver(mLaunchScreenReadyWhenReceiver);
        }
    }

    private void setReadyWhen(String readyWhen) {
        if (readyWhen.equals("first-paint")) {
            mReadyWhen = ReadyWhenType.FIRST_PAINT;
        } else if (readyWhen.equals("user-interactive")) {
            mReadyWhen = ReadyWhenType.USER_INTERACTIVE;
        } else if (readyWhen.equals("complete")) {
            mReadyWhen = ReadyWhenType.COMPLETE;
        } else if (readyWhen.equals("custom")) {
            mReadyWhen = ReadyWhenType.CUSTOM;
        } else {
            mReadyWhen = ReadyWhenType.FIRST_PAINT;
        }
    }
}
