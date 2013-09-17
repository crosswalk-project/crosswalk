// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension.api.presentation;

import android.content.Context;
import android.util.JsonWriter;
import android.util.Log;
import android.view.Display;

import java.io.IOException;
import java.io.StringWriter;

import org.xwalk.runtime.extension.api.XWalkDisplayManager;
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

    private XWalkDisplayManager mDisplayManager;

    // The number of available presentation displays on the system.
    private int mAvailableDisplayCount = 0;

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
    private final XWalkDisplayManager.DisplayListener mDisplayListener =
            new XWalkDisplayManager.DisplayListener() {
        @Override
        public void onDisplayAdded(int displayId) {
            ++mAvailableDisplayCount;

            // Notify that the secondary display for presentation show becomes
            // available now if the first one is added.
            if (mAvailableDisplayCount == 1) notifyAvailabilityChanged(true);
        }

        @Override
        public void onDisplayRemoved(int displayId) {
            --mAvailableDisplayCount;

            // Notify that the secondary display for presentation show becomes
            // unavailable now if the last one is removed already.
            if (mAvailableDisplayCount == 0) notifyAvailabilityChanged(false);
        }

        @Override
        public void onDisplayChanged(int displayId) {
            // TODO(hmin): Figure out the behaviour when the display is changed.
        }
    };

    public PresentationExtension(String name, String jsApi, XWalkExtensionContext context) {
        super(name, jsApi, context);

        mDisplayManager = XWalkDisplayManager.getInstance(context.getContext());
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

    @Override
    public void onMessage(int instanceId, String message) {
        // TODO(hmin): handle the message received from the JS side.
    }

    @Override
    public String onSyncMessage(int instanceId, String message) {
        if (message.equals(CMD_QUERY_DISPLAY_AVAILABILITY)) {
            return mAvailableDisplayCount != 0 ? "true" : "false";
        } else {
            Log.e(TAG, "Unexpected sync message received: " + message);
            return "";
        }
    }

    @Override
    public void onResume() {
        Display[] displays = mDisplayManager.getPresentationDisplays();

        // If there was available displays but right now no one is available for presentation,
        // we need to notify the display availability changes and reset the display count.
        if (displays.length == 0 && mAvailableDisplayCount > 0) {
            notifyAvailabilityChanged(false);
            mAvailableDisplayCount = 0;
        }

        // If there was no available display but right now there is at least one available
        // display, we need to notify the display availability changes and update the display
        // count.
        if (displays.length > 0 && mAvailableDisplayCount == 0) {
            notifyAvailabilityChanged(true);
            mAvailableDisplayCount = displays.length;
        }

        // If there was available displays and right now there is also at least one
        // available display, we only need to update the display count.
        if (displays.length > 0 && mAvailableDisplayCount > 0) {
            mAvailableDisplayCount = displays.length;
        }

        // Register the listener to display manager.
        mDisplayManager.registerDisplayListener(mDisplayListener);
    }

    @Override
    public void onPause() {
        mDisplayManager.unregisterDisplayListener(mDisplayListener);
    }

    @Override
    public void onDestroy() {
    }
}
