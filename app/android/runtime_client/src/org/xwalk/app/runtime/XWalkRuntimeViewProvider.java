// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app.runtime;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.view.View;

/**
 * The interface to provide the bridge between XWalkRuntimeView and the
 * real implementation like runtime core.
 *
 */
interface XWalkRuntimeViewProvider {
    // For handling life cycle and activity result.
    public void onCreate();
    public void onResume();
    public void onPause();
    public void onDestroy();
    public boolean onNewIntent(Intent intent);

    // For RuntimeView APIs.
    public String getVersion();
    public View getView();
    public void loadAppFromUrl(String url);
    public void loadAppFromManifest(String manifestUrl);
    public void enableRemoteDebugging(String frontEndUrl, String socketName);
    public void disableRemoteDebugging();
    public void loadExtensions();

    // For instrumentation test.
    public String getTitleForTest();
    public void setCallbackForTest(Object callback);
    public void loadDataForTest(String data, String mimeType, boolean isBase64Encoded);
}
