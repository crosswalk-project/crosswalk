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
import android.net.Uri;
import android.os.Bundle;
import android.view.View;

import org.xwalk.app.runtime.extension.XWalkRuntimeExtensionManager;
import org.xwalk.app.runtime.XWalkRuntimeLibraryException;
import org.xwalk.app.runtime.XWalkRuntimeView;
import org.xwalk.core.ReflectionHelper;
import org.xwalk.core.XWalkPreferences;

public abstract class XWalkRuntimeActivityBase extends Activity {

    private static final String DEFAULT_LIBRARY_APK_URL = null;

    private XWalkRuntimeView mRuntimeView;

    private boolean mShownNotFoundDialog = false;

    private BroadcastReceiver mReceiver;

    private boolean mRemoteDebugging = false;

    private AlertDialog mLibraryNotFoundDialog = null;

    private XWalkRuntimeExtensionManager mExtensionManager;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        IntentFilter intentFilter = new IntentFilter("org.xwalk.intent");
        intentFilter.addAction("android.intent.action.EXTERNAL_APPLICATIONS_AVAILABLE");
        intentFilter.addAction("android.intent.action.EXTERNAL_APPLICATIONS_UNAVAILABLE");
        mReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                Bundle bundle = intent.getExtras();
                if (bundle == null)
                    return;

                if (bundle.containsKey("remotedebugging")) {
                    String extra = bundle.getString("remotedebugging");
                    if (extra.equals("true")) {
                        XWalkPreferences.setValue(XWalkPreferences.REMOTE_DEBUGGING, true);
                    } else if (extra.equals("false")) {
                        XWalkPreferences.setValue(XWalkPreferences.REMOTE_DEBUGGING, false);
                    }
                }
            }
        };
        registerReceiver(mReceiver, intentFilter);
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
        unregisterReceiver(mReceiver);
        if (mExtensionManager != null) mExtensionManager.onDestroy();
        super.onDestroy();
    }

    @Override
    public void onNewIntent(Intent intent) {
        if (mRuntimeView == null || !mRuntimeView.onNewIntent(intent)) super.onNewIntent(intent);
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
            mRuntimeView = new XWalkRuntimeView(this, this, null);
            mShownNotFoundDialog = false;
            if (mLibraryNotFoundDialog != null) mLibraryNotFoundDialog.cancel();
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
        if (e instanceof RuntimeException) {
            handleException(e.getCause());
            return;
        }

        if (e instanceof XWalkRuntimeLibraryException) {
            String title = "";
            String message = "";
            XWalkRuntimeLibraryException runtimeException = (XWalkRuntimeLibraryException) e;
            switch (runtimeException.getType()) {
            case XWalkRuntimeLibraryException.XWALK_RUNTIME_LIBRARY_NOT_UP_TO_DATE_CRITICAL:
                title = getString("dialog_title_update_runtime_lib");
                message = getString("dialog_message_update_runtime_lib");
                break;
            case XWalkRuntimeLibraryException.XWALK_RUNTIME_LIBRARY_NOT_UP_TO_DATE_WARNING:
                title = getString("dialog_title_update_runtime_lib_warning");
                message = getString("dialog_message_update_runtime_lib_warning");
                break;
            case XWalkRuntimeLibraryException.XWALK_RUNTIME_LIBRARY_NOT_INSTALLED:
                title = getString("dialog_title_install_runtime_lib");
                message = getString("dialog_message_install_runtime_lib");
                break;
            case XWalkRuntimeLibraryException.XWALK_RUNTIME_LIBRARY_INVOKE_FAILED:
            default:
                Exception originException = runtimeException.getOriginException();
                if (originException != null) handleException(originException);
                return;
            }
            showRuntimeLibraryExceptionDialog(title, message);
        } else {
            e.printStackTrace();
            throw new RuntimeException(e);
        }
    }

    private void showRuntimeLibraryExceptionDialog(String title, String message) {
        if (!mShownNotFoundDialog) {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            if (!ReflectionHelper.shouldUseLibrary()) {
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

}
