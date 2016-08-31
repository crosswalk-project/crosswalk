// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app.runtime;

import android.content.Context;
import android.content.Intent;
import android.util.AttributeSet;
import android.widget.LinearLayout;

/**
 * This class is to provide public APIs which are called by web application
 * APKs. Since the runtime is shared as a library APK, web application APKs
 * have to call them via class loader but not direct API calling.
 *
 * A web application APK should create its Activity and set this view as
 * its content view.
 */
// Implementation notes.
// Please be careful to change any public APIs for the backward compatibility
// is very important to us. Don't change any of them without permisson.
public class XWalkRuntimeView extends LinearLayout {
    // The actual implementation to hide the internals to API users.
    private XWalkRuntimeViewProvider mProvider;

    /**
     * This is for inflating this view from XML. Called from test shell.
     * @param context a context to construct View
     * @param attrs the attributes of the XML tag that is inflating the view
     */
    public XWalkRuntimeView(Context context, AttributeSet attrs) {
        super(context, attrs);

        mProvider = XWalkRuntimeViewProviderFactory.getProvider(context);
        setOrientation(LinearLayout.VERTICAL);
    }
    /**
     * Tell runtime that the application is on creating. This can make runtime
     * be aware of application life cycle.
     */
    public void onCreate() {
        mProvider.onCreate();
        addView(mProvider.getView(), new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.MATCH_PARENT));
    }

    /**
     * Tell runtime that the application is on start. This can make runtime
     * be aware of application life cycle.
     */
    public void onStart() {
        mProvider.onStart();
    }

    /**
     * Tell runtime that the application is on resuming. This can make runtime
     * be aware of application life cycle.
     */
    public void onResume() {
        mProvider.onResume();
    }

    /**
     * Tell runtime that the application is on pausing. This can make runtime
     * be aware of application life cycle.
     */
    public void onPause() {
        mProvider.onPause();
    }

    /**
     * Tell runtime that the application is on stop. This can make runtime
     * be aware of application life cycle.
     */
    public void onStop() {
        mProvider.onStop();
    }

    /**
     * Tell runtime that the application is on destroying. This can make runtime
     * be aware of application life cycle.
     */
    public void onDestroy() {
        mProvider.onDestroy();
    }

    /**
     * Tell runtime that the activity receive a new Intent to start it. It may contains
     * data that runtime want to deal with.
     * @param intent the new coming Intent.
     * @return boolean whether runtime consumed it.
     */
    public boolean onNewIntent(Intent intent) {
        return mProvider.onNewIntent(intent);
    }

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        mProvider.onActivityResult(requestCode, resultCode, data);
    }

    /**
     * Get the version information of current runtime library.
     *
     * @return the string containing the version information.
     */
    public String getVersion() {
        return mProvider.getVersion();
    }

    /**
     * Load a web application through the entry url. It may be
     * a file from assets or a url from network.
     *
     * @param url the url of loaded html resource.
     */
    public void loadAppFromUrl(String url) {
        mProvider.loadAppFromUrl(url);
    }

    /**
     * Load a web application through the url of the manifest file.
     * The manifest file typically is placed in android assets. Now it is
     * compliant to W3C SysApps spec.
     *
     * @param manifestUrl the url of the manifest file
     */
    public void loadAppFromManifest(String manifestUrl) {
        mProvider.loadAppFromManifest(manifestUrl);
    }

    public void setRemoteDebugging(boolean value) {
        mProvider.setRemoteDebugging(value);
    }

    public void setUseAnimatableView(boolean value) {
        mProvider.setUseAnimatableView(value);
    }

    // For instrumentation test.
    public String getTitleForTest() {
        return mProvider.getTitleForTest();
    }

    public void setCallbackForTest(Object callback) {
        mProvider.setCallbackForTest(callback);
    }

    public void loadDataForTest(String data, String mimeType, boolean isBase64Encoded) {
        mProvider.loadDataForTest(data, mimeType, isBase64Encoded);
    }
}
