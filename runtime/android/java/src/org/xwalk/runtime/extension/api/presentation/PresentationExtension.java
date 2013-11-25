// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension.api.presentation;

import android.content.Context;
import android.os.Build;
import android.os.Bundle;
import android.util.JsonReader;
import android.util.JsonWriter;
import android.util.Log;
import android.view.Display;
import android.view.ViewGroup;

import java.io.IOException;
import java.io.StringReader;
import java.io.StringWriter;
import java.net.URI;
import java.net.URISyntaxException;

import org.chromium.base.ThreadUtils;

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
    private final static String TAG_BASE_URL = "baseUrl";
    private final static String TAG_CMD = "cmd";
    private final static String TAG_DATA = "data";
    private final static String TAG_REQUEST_ID = "requestId";
    private final static String TAG_URL = "url";

    // Command messages:
    private final static String CMD_DISPLAY_AVAILABLE_CHANGE = "DisplayAvailableChange";
    private final static String CMD_QUERY_DISPLAY_AVAILABILITY = "QueryDisplayAvailability";
    private final static String CMD_REQUEST_SHOW = "RequestShow";
    private final static String CMD_SHOW_SUCCEEDED = "ShowSucceeded";
    private final static String CMD_SHOW_FAILED = "ShowFailed";

    // Error messages:
    private final static String ERROR_INVALID_ACCESS = "InvalidAccessError";
    private final static String ERROR_INVALID_PARAMETER = "InvalidParameterError";
    private final static String ERROR_INVALID_STATE = "InvalidStateError";
    private final static String ERROR_NOT_FOUND = "NotFoundError";
    private final static String ERROR_NOT_SUPPORTED = "NotSupportedError";

    private XWalkDisplayManager mDisplayManager;

    // The number of available presentation displays on the system.
    private int mAvailableDisplayCount = 0;

    // The presentation content and view to be showed on the secondary display.
    // Currently, only one presentation is allowed to show at the same time.
    private XWalkPresentationContent mPresentationContent;
    private XWalkPresentationContent.PresentationDelegate mPresentationDelegate;
    private PresentationView mPresentationView;

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
            if (mAvailableDisplayCount == 0) {
                notifyAvailabilityChanged(false);
                // Destroy the presentation content if there is no available secondary display
                // any more.
                if (mPresentationContent != null) {
                    mPresentationContent.close();
                    mPresentationContent = null;
                }
            }
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

    private Display getPreferredDisplay() {
        Display[] displays = mDisplayManager.getPresentationDisplays();
        if (displays.length > 0) return displays[0];
        else return null;
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

    private void notifyRequestShowSucceed(int instanceId, int requestId, int presentationId) {
        StringWriter contents = new StringWriter();
        JsonWriter writer = new JsonWriter(contents);

        try {
            writer.beginObject();
            writer.name(TAG_CMD).value(CMD_SHOW_SUCCEEDED);
            writer.name(TAG_REQUEST_ID).value(requestId);
            writer.name(TAG_DATA).value(presentationId);
            writer.endObject();
            writer.close();

            postMessage(instanceId, contents.toString());
        } catch (IOException e) {
            Log.e(TAG, "Error: " + e.toString());
        }
    }

    private void notifyRequestShowFail(int instanceId, int requestId, String errorMessage) {
        StringWriter contents = new StringWriter();
        JsonWriter writer = new JsonWriter(contents);

        try {
            writer.beginObject();
            writer.name(TAG_CMD).value(CMD_SHOW_FAILED);
            writer.name(TAG_REQUEST_ID).value(requestId);
            writer.name(TAG_DATA).value(errorMessage);
            writer.endObject();
            writer.close();

            postMessage(instanceId, contents.toString());
        } catch (IOException e) {
            Log.e(TAG, "Error: " + e.toString());
        }
    }

    @Override
    public void onMessage(int instanceId, String message) {
        StringReader contents = new StringReader(message);
        JsonReader reader = new JsonReader(contents);

        int requestId = -1;
        String cmd = null;
        String url = null;
        String baseUrl = null;
        try {
            reader.beginObject();
            while (reader.hasNext()) {
                String name = reader.nextName();
                if (name.equals(TAG_CMD)) {
                    cmd = reader.nextString();
                } else if (name.equals(TAG_REQUEST_ID)) {
                    requestId = reader.nextInt();
                } else if (name.equals(TAG_URL)) {
                    url = reader.nextString();
                } else if (name.equals(TAG_BASE_URL)) {
                    baseUrl = reader.nextString();
                } else {
                    reader.skipValue();
                }
            }
            reader.endObject();
            reader.close();

            if (cmd != null && cmd.equals(CMD_REQUEST_SHOW) && requestId >= 0) {
                handleRequestShow(instanceId, requestId, url, baseUrl);
            }
        } catch (IOException e) {
            Log.d(TAG, "Error: " + e);
        }
    }

    private void handleRequestShow(final int instanceId, final int requestId,
                                   final String url, final String baseUrl) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN_MR1) {
            notifyRequestShowFail(instanceId, requestId, ERROR_NOT_SUPPORTED);
            return;
        }

        if (mAvailableDisplayCount == 0) {
            Log.d(TAG, "No available presentation display is found.");
            notifyRequestShowFail(instanceId, requestId, ERROR_NOT_FOUND);
            return;
        }

        // We have to post the runnable task for presentation view creation to
        // UI thread since extension API is not running on UI thread.
        ThreadUtils.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Display preferredDisplay = getPreferredDisplay();

                // No availble display is found.
                if (preferredDisplay == null) {
                    notifyRequestShowFail(instanceId, requestId, ERROR_NOT_FOUND);
                    return;
                }

                // Only one presentation is allowed to show on the presentation display. Notify
                // the JS side that an error occurs if there is already one presentation showed.
                if (mPresentationContent != null) {
                    notifyRequestShowFail(instanceId, requestId, ERROR_INVALID_ACCESS);
                    return;
                }

                // Check the url passed to requestShow.
                // If it's relative, combine it with baseUrl to make it abslute.
                // If the url is invalid, notify the JS side ERROR_INVALID_PARAMETER exception.
                String targetUrl = url;
                URI targetUri = null;
                try {
                    targetUri = new URI(url);
                    if (!targetUri.isAbsolute()) {
                        URI baseUri = new URI(baseUrl);
                        targetUrl = baseUri.resolve(targetUri).toString();
                    }
                } catch (URISyntaxException e) {
                    Log.e(TAG, "Invalid url passed to requestShow");
                    notifyRequestShowFail(instanceId, requestId, ERROR_INVALID_PARAMETER);
                    return;
                }

                mPresentationContent = new XWalkPresentationContent(
                        mExtensionContext.getContext(),
                        mExtensionContext.getActivity(),
                        new XWalkPresentationContent.PresentationDelegate() {
                    @Override
                    public void onContentLoaded(XWalkPresentationContent content) {
                        notifyRequestShowSucceed(instanceId, requestId, content.getPresentationId());
                    }

                    @Override
                    public void onContentClosed(XWalkPresentationContent content) {
                        if (content == mPresentationContent) {
                            mPresentationContent.close();
                            mPresentationContent = null;
                            if (mPresentationView != null) mPresentationView.cancel();
                        }
                    }
                });

                // Start to load the content from the target url.
                mPresentationContent.load(targetUrl);

                // Update the presentation view in order that the content could be presented
                // on the preferred display.
                updatePresentationView(preferredDisplay);
            }
        });
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
            if (mPresentationContent != null) {
                mPresentationContent.close();
                mPresentationContent = null;
            }
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

        if (mPresentationContent != null) {
            mPresentationContent.onResume();
        }

        updatePresentationView(getPreferredDisplay());

        // Register the listener to display manager.
        mDisplayManager.registerDisplayListener(mDisplayListener);
    }

    private void updatePresentationView(Display preferredDisplay) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN_MR1 ||
                preferredDisplay == null) {
            return;
        }

        // No presentation content is ready for use.
        if (mPresentationView == null && mPresentationContent == null) {
            return;
        }

        // If the presentation view is showed on another display, we need to dismiss it
        // and re-create a new one.
        if (mPresentationView != null && mPresentationView.getDisplay() != preferredDisplay) {
            mPresentationView.dismiss();
            mPresentationView = null;
        }

        // If the presentation view is not NULL and its associated display is not changed,
        // the displaying system will automatically restore the content on the view once
        // the Activity gets resumed.
        if (mPresentationView == null && mPresentationContent != null) {
            // Remove the content view from its previous view hierarchy if have.
            ViewGroup parent = (ViewGroup)mPresentationContent.getContentView().getParent();
            if (parent != null) parent.removeView(mPresentationContent.getContentView());

            mPresentationView = PresentationView.createInstance(mExtensionContext.getContext(), preferredDisplay);
            mPresentationView.setContentView(mPresentationContent.getContentView());
            mPresentationView.setPresentationListener(new PresentationView.PresentationListener() {
                @Override
                public void onDismiss(PresentationView view) {
                    // We need to pause the content if the view is dismissed from the screen
                    // to avoid unnecessary overhead to update the content, e.g. stop animation
                    // and JS execution.
                    if (view == mPresentationView) {
                        if (mPresentationContent != null) mPresentationContent.onPause();
                        mPresentationView = null;
                    }
                }

                @Override
                public void onShow(PresentationView view) {
                    // The presentation content may be paused due to the presentation view was
                    // dismissed, we need to resume it when the new view is showed.
                    if (view == mPresentationView && mPresentationContent != null) {
                        mPresentationContent.onResume();
                    }
                }
            });
        }

        mPresentationView.show();
    }

    @Override
    public void onPause() {
        if (mPresentationView != null) {
            mPresentationView.dismiss();
            mPresentationView = null;
        }

        if (mPresentationContent != null) mPresentationContent.onPause();

        // No need to listen display changes when the activity is paused.
        mDisplayManager.unregisterDisplayListener(mDisplayListener);
    }

    @Override
    public void onDestroy() {
    }
}
