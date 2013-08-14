// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime;

import android.content.Context;
import android.content.Intent;
import android.view.View;

/**
 * The interface to provide the bridge between XWalkRuntimeView and the
 * real implementation.
 */
interface XWalkRuntimeViewProvider {
    public View getView();
    public void loadAppFromUrl(String url);
    public void loadAppFromManifest(String manifestUrl);
    public void onCreate();
    public void onResume();
    public void onPause();
    public void onDestroy();
    public void onActivityResult(int requestCode, int resultCode, Intent data);
    public String enableRemoteDebugging(String frontEndUrl, String socketName);
    public void disableRemoteDebugging();
}
