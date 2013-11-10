// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension.api;

import android.content.Context;
import android.hardware.display.DisplayManager;
import android.view.Display;

/**
 * A wrapper class for DisplayManager implementation on Android JellyBean MR1 (API Level 17).
 */
public class DisplayManagerJBMR1 extends XWalkDisplayManager implements DisplayManager.DisplayListener {
    private DisplayManager mDisplayManager;

    public DisplayManagerJBMR1(Context context) {
        mDisplayManager = (DisplayManager) context.getSystemService(Context.DISPLAY_SERVICE);
    }

    @Override
    public Display getDisplay(int displayId) {
        return mDisplayManager.getDisplay(displayId);
    }

    @Override
    public Display[] getDisplays() {
        return mDisplayManager.getDisplays();
    }

    @Override
    public Display[] getPresentationDisplays() {
        String category = DisplayManager.DISPLAY_CATEGORY_PRESENTATION;
        return mDisplayManager.getDisplays(category);
    }

    @Override
    public void registerDisplayListener(XWalkDisplayManager.DisplayListener listener) {
        super.registerDisplayListener(listener);
        if (mListeners.size() == 1)
            mDisplayManager.registerDisplayListener(this, null);
    }

    @Override
    public void unregisterDisplayListener(XWalkDisplayManager.DisplayListener listener) {
        super.unregisterDisplayListener(listener);
        if (mListeners.size() == 0)
            mDisplayManager.unregisterDisplayListener(this);
    }

    @Override
    public void onDisplayAdded(int displayId) {
        notifyDisplayAdded(displayId);
    }

    @Override
    public void onDisplayRemoved(int displayId) {
        notifyDisplayRemoved(displayId);
    }

    @Override
    public void onDisplayChanged(int displayId) {
        notifyDisplayChanged(displayId);
    }
}

