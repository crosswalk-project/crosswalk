// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension.api;

import android.os.Build;
import android.content.Context;
import android.view.Display;
import java.util.ArrayList;

/**
 * A helper class to abstract the display manager for different Android build version.
 */
public abstract class XWalkDisplayManager {
    protected final ArrayList<DisplayListener> mListeners = new ArrayList<DisplayListener>();
    private static XWalkDisplayManager mInstance;
    // Hold the context of single and global application object of the current process.
    private static Context mContext;

    /**
     * Return the singleton DisplayManager instance for different android build. 
     *
     * TODO(hmin): Need to make it thread-safe.
     *
     * @param context The given application context.
     */
    public static XWalkDisplayManager getInstance(Context context) {
        if (mContext != null) {
            // Would never be happened.
            assert context.getApplicationContext() == mContext;
        } else {
            mContext = context.getApplicationContext();
        }

        if (mInstance == null) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1)
                mInstance = new DisplayManagerJBMR1(mContext);
            else
                mInstance = new DisplayManagerNull();
        }
        return mInstance;
    }

    /**
     * Get information about a logical display.
     */
    public abstract Display getDisplay(int displayId);

    /**
     * Get all currently valid logical displays, including the built-in display.
     */
    public abstract Display[] getDisplays();

    /**
     * Get all valid secondary displays, excluding the built-in display. The returned array
     * is sorted for preference. The first display in the returned array is the most preferred
     * display for presentation show.
     */
    public abstract Display[] getPresentationDisplays();

    public void registerDisplayListener(DisplayListener listener) {
        mListeners.add(listener);
    }

    public void unregisterDisplayListener(DisplayListener listener) {
        mListeners.remove(listener);
    }

    protected void notifyDisplayAdded(int displayId) {
        for (DisplayListener listener : mListeners)
            listener.onDisplayAdded(displayId);
    }

    protected void notifyDisplayRemoved(int displayId) {
        for (DisplayListener listener : mListeners)
            listener.onDisplayRemoved(displayId);
    }

    protected void notifyDisplayChanged(int displayId) {
        for (DisplayListener listener : mListeners)
            listener.onDisplayChanged(displayId);
    }


    /**
     * Listen for display arrival, removal and change event.
     */
    public interface DisplayListener {
        /**
         * Called whenever a logical display has been added to the system.
         */
        public void onDisplayAdded(int displayId);

        /**
         * Called whenever a logical display has been removed to the system.
         */
        public void onDisplayRemoved(int displayId);

        /**
         * Called whenever a logical display has been changed
         */
        public void onDisplayChanged(int displayId);
    }
}
