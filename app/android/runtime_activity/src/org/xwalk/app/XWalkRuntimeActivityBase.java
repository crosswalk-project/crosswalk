// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

import org.xwalk.app.runtime.extension.XWalkRuntimeExtensionManager;
import org.xwalk.app.runtime.XWalkRuntimeView;
import org.xwalk.core.SharedXWalkExceptionHandler;
import org.xwalk.core.SharedXWalkView;
import org.xwalk.core.XWalkActivity;
import org.xwalk.core.XWalkPreferences;

public abstract class XWalkRuntimeActivityBase extends XWalkActivity {

    private static final String DEFAULT_LIBRARY_APK_URL = null;

    private static final String TAG = "XWalkRuntimeActivityBase";

    private XWalkRuntimeView mRuntimeView;

    private boolean mShownNotFoundDialog = false;

    private boolean mRemoteDebugging = false;

    private boolean mUseAnimatableView = false;

    private AlertDialog mLibraryNotFoundDialog = null;

    private XWalkRuntimeExtensionManager mExtensionManager;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        tryLoadRuntimeView();
        if (mRuntimeView != null) mRuntimeView.onCreate();
    }

    @Override
    public void onStart() {
        super.onStart();
        if (mExtensionManager != null) mExtensionManager.onStart();
    }

    @Override
    public void onPause() {
        super.onPause();
        if (mRuntimeView != null) mRuntimeView.onPause();
        if (mExtensionManager != null) mExtensionManager.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
        if (mRuntimeView != null) mRuntimeView.onResume();
        if (mExtensionManager != null) mExtensionManager.onResume();
    }

    @Override
    public void onStop() {
        super.onStop();
        if (mExtensionManager != null) mExtensionManager.onStop();
    }

    @Override
    public void onDestroy() {
        if (mExtensionManager != null) mExtensionManager.onDestroy();
        super.onDestroy();
    }

    @Override
    public void onNewIntent(Intent intent) {
        if (mRuntimeView == null || !mRuntimeView.onNewIntent(intent)) super.onNewIntent(intent);
        if (mExtensionManager != null) mExtensionManager.onNewIntent(intent);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (mRuntimeView != null) mRuntimeView.onActivityResult(requestCode, resultCode, data);
        if (mExtensionManager != null) mExtensionManager.onActivityResult(requestCode, resultCode, data);
    }

    private String getLibraryApkDownloadUrl() {
        int resId = getResources().getIdentifier("xwalk_library_apk_download_url", "string", getPackageName());
        if (resId == 0) return DEFAULT_LIBRARY_APK_URL;
        return getString(resId);
    }

    public String getString(String label) {
        int resId = getResources().getIdentifier(label, "string", getPackageName());
        if (resId == 0) return label.replace('_', ' ');
        return getString(resId);
    }

    private void tryLoadRuntimeView() {
        try {
            SharedXWalkView.initialize(this, new SharedXWalkExceptionHandler() {
                @Override
                public void onSharedLibraryNotFound() {
                    String title = getString("dialog_title_install_runtime_lib");
                    String message = getString("dialog_message_install_runtime_lib");
                    showRuntimeLibraryExceptionDialog(title, message);
                }
            });
            if (mUseAnimatableView) {
                XWalkPreferences.setValue(XWalkPreferences.ANIMATABLE_XWALK_VIEW, true);
            } else {
                XWalkPreferences.setValue(XWalkPreferences.ANIMATABLE_XWALK_VIEW, false);
            }
            mShownNotFoundDialog = false;
            if (mLibraryNotFoundDialog != null) mLibraryNotFoundDialog.cancel();
            mRuntimeView = new XWalkRuntimeView(this, this, null);
            if (mRemoteDebugging) {
                XWalkPreferences.setValue(XWalkPreferences.REMOTE_DEBUGGING, true);
            } else {
                XWalkPreferences.setValue(XWalkPreferences.REMOTE_DEBUGGING, false);
            }
            // XWalkPreferences.ENABLE_EXTENSIONS
            if (XWalkPreferences.getValue("enable-extensions")) {
                // Enable xwalk extension mechanism and start load extensions here.
                // Note that it has to be after above initialization.
                mExtensionManager = new XWalkRuntimeExtensionManager(getApplicationContext(), this);
                mExtensionManager.loadExtensions();
            }
        } catch (Exception e) {
            handleException(e);
        }
        didTryLoadRuntimeView(mRuntimeView);
    }

    public XWalkRuntimeView getRuntimeView() {
        return mRuntimeView;
    }

    public void handleException(Throwable e) {
        if (e == null) return;
        if (e instanceof RuntimeException && e.getCause() != null) {
            handleException(e.getCause());
            return;
        }
        Log.e(TAG, Log.getStackTraceString(e));
    }

    private void showRuntimeLibraryExceptionDialog(String title, String message) {
        if (!mShownNotFoundDialog) {
            if (SharedXWalkView.isUsingLibrary()) return;
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            if (SharedXWalkView.containsLibrary()) {
                builder.setPositiveButton(android.R.string.ok,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int id) {
                                finish();
                            }
                        });
            } else {
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
                                goToMarket.setData(Uri.parse(
                                        "market://details?id=org.xwalk.core"));
                                startActivity(goToMarket);
                            }
                        });
            }
            builder.setTitle(title).setMessage(message);

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

    public void setUseAnimatableView(boolean value) {
        mUseAnimatableView = value;
    }

}
