// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension.api.presentation;

import android.content.Context;
import android.hardware.display.DisplayManager;
import android.util.JsonWriter;
import android.util.Log;
import android.util.SparseArray;
import android.view.Display;

import java.io.IOException;
import java.io.StringWriter;
import java.util.ArrayList;

import org.xwalk.runtime.extension.XWalkExtension;
import org.xwalk.runtime.extension.XWalkExtensionContext;

/**
 * A XWalk extension for Presentation API implementation on Android.
 */
public class PresentationExtension extends XWalkExtension {
    public final static String TAG = "PresentationExtension";
    public final static String JS_API_PATH = "jsapi/presentation_api.js";
    public final static String NAME = "navigator.presentation";

    // Tags:
    private final static String TAG_CMD = "cmd";
    private final static String TAG_DATA = "data";

    // Command messages.
    private final static String CMD_DISPLAY_AVAILABLE_CHANGE = "DisplayAvailableChange";
    private final static String CMD_QUERY_DISPLAY_AVAILABILITY = "QueryDisplayAvailability";

    private DisplayManager mDisplayManager;

    // Holds all available presentation displays connected to the system.
    private final SparseArray<Display> mDisplayList = new SparseArray<Display>();

    /**
     * Listens for the secondary display arrival and removal.
     *
     * We rely on onDisplayAdded/onDisplayRemoved callback to trigger the display
     * availability change event. The presentation display becomes available if
     * the first secondary display is arrived, and becomes unavailable if one
     * of the last secondary display is removed.
     *
     * Note the display id is a system-wide unique number for each physical connection.
     * It means that for the same display device, the display id assigned by the system
     * would be different if it is re-connected again.
     */
    private final DisplayManager.DisplayListener mDisplayListener =
            new DisplayManager.DisplayListener() {
        @Override
        public void onDisplayAdded(int displayId) {
            Display display = mDisplayManager.getDisplay(displayId);
            addSecondaryDisplay(displayId, display);
        }

        @Override
        public void onDisplayRemoved(int displayId) {
            removeSecondaryDisplay(displayId);
        }

        @Override
        public void onDisplayChanged(int displayId) {
            // TODO(hmin): Figure out the behaviour when the display is changed.
        }
    };

    public PresentationExtension(String name, String jsApi, XWalkExtensionContext context) {
        super(name, jsApi, context);

        mDisplayManager =
                (DisplayManager) context.getContext().getSystemService(Context.DISPLAY_SERVICE);
    }

    private String getDisplayCategory() {
        // With DISPLAY_CATEGORY_PRESENTATION category, the first display in the
        // returned display array is the most preferred presentation display
        // sorted by display manager. Note this category excludes the built-in
        // primary display.
        return DisplayManager.DISPLAY_CATEGORY_PRESENTATION;
    }

    private void notifyAvailabilityChanged(boolean isAvailable) {
        StringWriter contents = new StringWriter();
        JsonWriter writer = new JsonWriter(contents);

        try {
            writer.beginObject();
            writer.name(TAG_CMD).value(CMD_DISPLAY_AVAILABLE_CHANGE);
            writer.name(TAG_DATA).value(isAvailable);
            writer.endObject();
            writer.close();

            broadcastMessage(contents.toString());
        } catch (IOException e) {
            Log.e(TAG, "Error: " + e.toString());
        }
    }

    public void addSecondaryDisplay(int displayId, Display display) {
        mDisplayList.put(displayId, display);

        // Notify that the secondary display for presentation show becomes
        // available now if the first one is added.
        if (mDisplayList.size() == 1) notifyAvailabilityChanged(true);
    }

    public void removeSecondaryDisplay(int displayId) {
        mDisplayList.remove(displayId);

        // Notify that the secondary display for presentation show becomes
        // unavailable now if the last one is removed already.
        if (mDisplayList.size() == 0) notifyAvailabilityChanged(false);
    }

    @Override
    public void onMessage(int instanceId, String message) {
        // TODO(hmin): handle the message received from the JS side.
    }

    @Override
    public String onSyncMessage(int instanceId, String message) {
        if (message.equals(CMD_QUERY_DISPLAY_AVAILABILITY)) {
            return mDisplayList.size() != 0 ? "true" : "false";
        } else {
            Log.e(TAG, "Unexpected sync message received: " + message);
            return "";
        }
    }

    @Override
    public void onResume() {
        Display[] displays = mDisplayManager.getDisplays(getDisplayCategory());

        // If no available presentation display is connected, and the cached
        // display list is not empty, we need to notify the display availabe
        // changes and clear the list.
        if (displays.length == 0 && mDisplayList.size() > 0) {
            notifyAvailabilityChanged(false);
            mDisplayList.clear();
        }

        // If there is at least one available presentation display connected,
        // and the cached display list is empty, we need to update the display
        // list and notify the display availability changes.
        if (displays.length > 0 && mDisplayList.size() == 0) {
            mDisplayList.clear();
            for (Display d : displays)
                addSecondaryDisplay(d.getDisplayId(), d);
        }

        // If there is at least one available presentation display connected,
        // and the cached display list is not empty, we only need to update
        // the display list.
        if (displays.length > 0 && mDisplayList.size() > 0) {
            mDisplayList.clear();
            for (Display d : displays)
                mDisplayList.put(d.getDisplayId(), d);
        }

        // Register the listener to display manager.
        mDisplayManager.registerDisplayListener(mDisplayListener, null);
    }

    @Override
    public void onPause() {
        mDisplayManager.unregisterDisplayListener(mDisplayListener);
    }

    @Override
    public void onDestroy() {
    }
}
