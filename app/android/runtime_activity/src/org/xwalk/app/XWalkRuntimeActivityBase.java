// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.net.Uri;
import android.os.Bundle;
import android.view.View;

import org.xwalk.app.runtime.CrossPackageWrapper;
import org.xwalk.app.runtime.CrossPackageWrapperExceptionHandler;
import org.xwalk.app.runtime.XWalkRuntimeClient;

public abstract class XWalkRuntimeActivityBase extends Activity implements CrossPackageWrapperExceptionHandler {

    private static final String DEFAULT_LIBRARY_APK_URL = null;

    private XWalkRuntimeClient mRuntimeView;

    private boolean mShownNotFoundDialog = false;

    private BroadcastReceiver mReceiver;

    private boolean mRemoteDebugging = false;

    private AlertDialog mLibraryNotFoundDialog = null;
    
    private XWalkMixedResources mMixedResources = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        IntentFilter intentFilter = new IntentFilter("org.xwalk.intent");
        mReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                Bundle bundle = intent.getExtras();
                if (bundle == null)
                    return;

                if (bundle.containsKey("remotedebugging")) {
                    String extra = bundle.getString("remotedebugging");
                    if (extra.equals("true")) {
                        String mPackageName = getApplicationContext().getPackageName();
                        mRuntimeView.enableRemoteDebugging("", mPackageName);
                    } else if (extra.equals("false")) {
                        mRuntimeView.disableRemoteDebugging();
                    }
                }
            }
        };
        registerReceiver(mReceiver, intentFilter);
        super.onCreate(savedInstanceState);
        tryLoadRuntimeView();
        mRuntimeView.onCreate();
    }

    @Override
    public void onStart() {
        super.onStart();
        tryLoadRuntimeView();
    }

    @Override
    public void onPause() {
        super.onPause();
        mRuntimeView.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
        mRuntimeView.onResume();
    }

    @Override
    public void onDestroy() {
        unregisterReceiver(mReceiver);
        super.onDestroy();
        mRuntimeView.onDestroy();
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        mRuntimeView.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    public Resources getResources() {
        if (mMixedResources == null) return super.getResources();
        return mMixedResources;
    }

    private String getLibraryApkDownloadUrl() {
        int resId = getResources().getIdentifier("xwalk_library_apk_download_url", "string", getPackageName());
        if (resId == 0) return DEFAULT_LIBRARY_APK_URL;
        return getString(resId);
    }

    public String getString(String label) {
        int resId = getResources().getIdentifier(label, "string", getPackageName());
        if (resId == 0) return label;
        return getString(resId);
    }

    private void tryLoadRuntimeView() {
        if (mRuntimeView == null || mRuntimeView.get() == null) {
            mRuntimeView = new XWalkRuntimeClient(this, null, this);
            if (mRuntimeView.get() != null) {
                mMixedResources = new XWalkMixedResources(super.getResources(),
                        mRuntimeView.getLibraryContext().getResources());
                mShownNotFoundDialog = false;
                if (mLibraryNotFoundDialog != null) mLibraryNotFoundDialog.cancel();
            }
            if (mRemoteDebugging) {
                String mPackageName = getApplicationContext().getPackageName();
                String result = mRuntimeView.enableRemoteDebugging("", mPackageName);
            } else {
                mRuntimeView.disableRemoteDebugging();
            }

            didTryLoadRuntimeView(mRuntimeView.get());
        }
    }

    public XWalkRuntimeClient getRuntimeView() {
        return mRuntimeView;
    }

    @Override
    public void onException(Exception e) {
        // TODO(wang16): Handle different kind of exception differently.
        showLibraryNotFoundDialog();
    }

    @Override
    public void onException(String msg) {
        showLibraryNotFoundDialog();
    }

    private void showLibraryNotFoundDialog() {
        if (!mShownNotFoundDialog) {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setNegativeButton(android.R.string.cancel,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            // User cancelled the dialog
                        }
                    });
            final String downloadUrl = getLibraryApkDownloadUrl();
            if (downloadUrl != null && downloadUrl.length() > 0) {
                builder.setNeutralButton(getString("download_from_url"),
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int id) {
                                Intent goDownload = new Intent(Intent.ACTION_VIEW);
                                goDownload.setData(Uri.parse(downloadUrl));
                                startActivity(goDownload);
                            }
                        });
            }
            builder.setPositiveButton(getString("download_from_store"),
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            Intent goToMarket = new Intent(Intent.ACTION_VIEW);
                            goToMarket.setData(Uri.parse("market://details?id="+CrossPackageWrapper.LIBRARY_APK_PACKAGE_NAME));
                            startActivity(goToMarket);
                        }
                    });
            builder.setTitle(getString("download_dialog_title")).setMessage(getString("download_dialog_msg"));

            mLibraryNotFoundDialog = builder.create();
            mLibraryNotFoundDialog.setOnCancelListener(new DialogInterface.OnCancelListener() {                
                @Override
                public void onCancel(DialogInterface dialog) {
                    mLibraryNotFoundDialog = null;
                }
            });
            mLibraryNotFoundDialog.setOnDismissListener(new DialogInterface.OnDismissListener() {                
                @Override
                public void onDismiss(DialogInterface dialog) {
                    mLibraryNotFoundDialog = null;
                }
            });
            mLibraryNotFoundDialog.show();
            mShownNotFoundDialog = true;
        }
    }

    /*
     * Called each time trying to load runtime view from library apk.
     * Descendant should handle both succeeded and failed to load
     * library apk.
     *
     * @param, The RuntimeView loaded, it can be null for failed to load RuntimeView.
     */
    abstract protected void didTryLoadRuntimeView(View runtimeView);

    public void setRemoteDebugging(boolean value) {
        mRemoteDebugging = value;
    }

}
