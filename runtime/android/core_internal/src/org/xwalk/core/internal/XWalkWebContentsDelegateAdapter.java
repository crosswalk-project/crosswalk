// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.content.Intent;
import android.net.Uri;
import android.util.Log;
import android.view.KeyEvent;

class XWalkWebContentsDelegateAdapter extends XWalkWebContentsDelegate {

    private XWalkContentsClient mXWalkContentsClient;

    public XWalkWebContentsDelegateAdapter(XWalkContentsClient client) {
        mXWalkContentsClient = client;
    }

    @Override
    public boolean shouldOpenWithDefaultBrowser(String contentUrl) {
        if (mXWalkContentsClient != null) {
            return mXWalkContentsClient.shouldOpenWithDefaultBrowser(contentUrl);
        }
        return false;
    }

    @Override
    public void onLoadProgressChanged(int progress) {
        if (mXWalkContentsClient != null) mXWalkContentsClient.onProgressChanged(progress);
    }

    @Override
    public boolean addNewContents(boolean isDialog, boolean isUserGesture) {
        // TODO: implement.
        return false;
    }

    @Override
    public void closeContents() {
        if (mXWalkContentsClient != null) mXWalkContentsClient.onCloseWindow();
    }

    @Override
    public void activateContents() {
        if (mXWalkContentsClient != null) mXWalkContentsClient.onRequestFocus();
    }

    @Override
    public void rendererUnresponsive() {
        if (mXWalkContentsClient != null) mXWalkContentsClient.onRendererUnresponsive();
    }

    @Override
    public void rendererResponsive() {
        if (mXWalkContentsClient != null) mXWalkContentsClient.onRendererResponsive();
    }

    @Override
    public void handleKeyboardEvent(KeyEvent event) {
        // Handle the event here when necessary and return if so.
        if (mXWalkContentsClient != null) mXWalkContentsClient.onUnhandledKeyEvent(event);
    }

    @Override
    public void toggleFullscreen(boolean enterFullscreen) {
        if (mXWalkContentsClient != null) mXWalkContentsClient.onToggleFullscreen(enterFullscreen);
    }

    @Override
    public boolean isFullscreen() {
        if (mXWalkContentsClient != null) return mXWalkContentsClient.hasEnteredFullscreen();

        return false;
    }

    @Override
    public boolean shouldOverrideRunFileChooser(int processId, int renderId, int mode,
            String acceptTypes, boolean capture) {
        if (mXWalkContentsClient != null) {
            return mXWalkContentsClient.shouldOverrideRunFileChooser(processId, renderId, mode,
                    acceptTypes, capture);
        }
        return false;
    }
}
