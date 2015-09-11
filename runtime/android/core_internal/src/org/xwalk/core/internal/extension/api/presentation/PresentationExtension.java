// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.extension.api.presentation;

import android.app.Activity;
import android.content.Context;
import android.os.Build;
import android.os.Bundle;
import android.provider.ContactsContract;
import android.util.JsonReader;
import android.util.JsonWriter;
import android.util.Log;
import android.view.Display;
import android.view.ViewGroup;

import java.io.IOException;
import java.io.StringReader;
import java.io.StringWriter;
import java.lang.ref.WeakReference;
import java.net.URI;
import java.net.URISyntaxException;

import org.chromium.base.ActivityState;
import org.chromium.base.ThreadUtils;

import org.xwalk.core.internal.extension.api.XWalkDisplayManager;
import org.xwalk.core.internal.extension.XWalkExtensionWithActivityStateListener;

/**
 * A XWalk extension for Presentation API implementation on Android.
 */
public class PresentationExtension extends XWalkExtensionWithActivityStateListener {
    public final static String JS_API_PATH = "jsapi/presentation_api.js";

    private final static String NAME = "navigator.presentation";
    private final static String TAG = "PresentationExtension";

    // Tags:
    private final static String TAG_BASE_URL = "baseUrl";
    private final static String TAG_CMD = "cmd";
    private final static String TAG_DATA = "data";
    private final static String TAG_REQUEST_ID = "requestId";
    private final static String TAG_PRESENTATION_ID = "presentationId";
    private final static String TAG_URL = "url";

    // Command messages:
    private final static String CMD_AVAILABILITY_CHANGE = "AvailabilityChange";
    private final static String CMD_GET_AVAILABILITY = "GetAvailability";
    private final static String CMD_START_SESSION = "StartSession";
    private final static String CMD_SEND_MESSAGE_TO_REMOTE_DISPLAY = "SendMessageToRemoteDisplay";
    private final static String CMD_SEND_MESSAGE_TO_HOST_DISPLAY = "SendMessageToHostDisplay";
    private final static String CMD_SESSION_START_SUCCEEDED = "SessionStartSucceeded";
    private final static String CMD_SESSION_START_FAILED = "SessionStartFailed";
    private final static String CMD_DEFAULT_SESSION_STARTED = "DefaultSessionStarted";
    private final static String CMD_SESSION_MESSAGE_TO_HOST_RECEIVED = "SessionMessageToHostReceived";
    private final static String CMD_SESSION_MESSAGE_TO_REMOTE_RECEIVED = "SessionMessageToRemoteReceived";

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
    private Context mContext;
    private WeakReference<Activity> mActivity;

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
                closePresentationContent();
            }
        }

        @Override
        public void onDisplayChanged(int displayId) {
            // TODO(hmin): Figure out the behaviour when the display is changed.
        }
    };

    public PresentationExtension(String jsApi, Activity activity) {
        super(NAME, jsApi, activity);

        mContext = activity.getApplicationContext();
        mActivity = new WeakReference<Activity>(activity);
        mDisplayManager = XWalkDisplayManager.getInstance(activity.getApplicationContext());
        Display[] displays = mDisplayManager.getPresentationDisplays();
        mAvailableDisplayCount = displays.length;
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
            writer.name(TAG_CMD).value(CMD_AVAILABILITY_CHANGE);
            writer.name(TAG_DATA).value(isAvailable);
            writer.endObject();
            writer.close();

            broadcastMessage(contents.toString());
        } catch (IOException e) {
            Log.e(TAG, "Error: " + e.toString());
        }
    }

    private void notifyStartSessionSucceed(int instanceId, int requestId, int presentationId) {
        StringWriter contents = new StringWriter();
        JsonWriter writer = new JsonWriter(contents);

        try {
            writer.beginObject();
            writer.name(TAG_CMD).value(CMD_SESSION_START_SUCCEEDED);
            writer.name(TAG_REQUEST_ID).value(requestId);
            writer.name(TAG_DATA).value(presentationId);
            writer.endObject();
            writer.close();

            postMessage(instanceId, contents.toString());
        } catch (IOException e) {
            Log.e(TAG, "Error: " + e.toString());
        }
    }

    private void notifyStartSessionFail(int instanceId, int requestId, String errorMessage) {
        StringWriter contents = new StringWriter();
        JsonWriter writer = new JsonWriter(contents);

        try {
            writer.beginObject();
            writer.name(TAG_CMD).value(CMD_SESSION_START_FAILED);
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
        int presentationId = -1;
        String data = null;
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
                } else if (name.equals(TAG_PRESENTATION_ID)) {
                    presentationId = reader.nextInt();
                } else if (name.equals(TAG_DATA)) {
                    data = reader.nextString();
                } else {
                    reader.skipValue();
                }
            }
            reader.endObject();
            reader.close();

            if (cmd == null) return;

            if (cmd.equals(CMD_START_SESSION) && requestId >= 0) {
                handleStartSession(instanceId, requestId, url, baseUrl);
            } else if (cmd.equals(CMD_SEND_MESSAGE_TO_REMOTE_DISPLAY)) {
                handleSendMessageToRemoteDisplay(instanceId, presentationId, data);
            } else if (cmd.equals(CMD_SEND_MESSAGE_TO_HOST_DISPLAY)) {
                handleSendMessageToHostDisplay(instanceId, presentationId, data);
            }
        } catch (IOException e) {
            Log.d(TAG, "Error: " + e);
        }
    }


    private void handleStartSession(final int instanceId, final int requestId,
                                    final String url, final String baseUrl) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN_MR1) {
            notifyStartSessionFail(instanceId, requestId, ERROR_NOT_SUPPORTED);
            return;
        }

        if (mAvailableDisplayCount == 0) {
            Log.d(TAG, "No available presentation display is found.");
            notifyStartSessionFail(instanceId, requestId, ERROR_NOT_FOUND);
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
                    notifyStartSessionFail(instanceId, requestId, ERROR_NOT_FOUND);
                    return;
                }

                // Only one presentation is allowed to show on the presentation display. Notify
                // the JS side that an error occurs if there is already one presentation showed.
                if (mPresentationContent != null) {
                    notifyStartSessionFail(instanceId, requestId, ERROR_INVALID_ACCESS);
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
                    notifyStartSessionFail(instanceId, requestId, ERROR_INVALID_PARAMETER);
                    return;
                }

                mPresentationContent = new XWalkPresentationContent(
                        mContext,
                        mActivity,
                        new XWalkPresentationContent.PresentationDelegate() {
                    @Override
                    public void onContentLoaded(XWalkPresentationContent content) {
                        notifyStartSessionSucceed(instanceId, requestId, content.getPresentationId());
                    }

                    @Override
                    public void onContentClosed(XWalkPresentationContent content) {
                        if (content == mPresentationContent) {
                            closePresentationContent();
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

    private void handleSendMessageToRemoteDisplay(final int instanceId, final int presentationId,
                                                  final String data) {
        notifySessionMessageReceived(false, presentationId, data);
    }

    private void handleSendMessageToHostDisplay(final int instanceId, final int presentationId,
                                                final String data) {
        notifySessionMessageReceived(true, presentationId, data);
    }

    private void notifySessionMessageReceived(final boolean isToHost, final int presentationId,
                                              final String data) {
        StringWriter contents = new StringWriter();
        JsonWriter writer = new JsonWriter(contents);

        try {
            writer.beginObject();
            if (isToHost) {
              writer.name(TAG_CMD).value(CMD_SESSION_MESSAGE_TO_HOST_RECEIVED);
            } else {
              writer.name(TAG_CMD).value(CMD_SESSION_MESSAGE_TO_REMOTE_RECEIVED);
            }
            writer.name(TAG_PRESENTATION_ID).value(presentationId);
            writer.name(TAG_DATA).value(data);
            writer.endObject();
            writer.close();

            broadcastMessage(contents.toString());
        } catch (IOException e) {
            Log.e(TAG, "Error: " + e.toString());
        }
    }

    @Override
    public String onSyncMessage(int instanceId, String message) {
        if (message.equals(CMD_GET_AVAILABILITY)) {
            return mAvailableDisplayCount != 0 ? "true" : "false";
        } else {
            Log.e(TAG, "Unexpected sync message received: " + message);
            return "";
        }
    }

    public void onResume() {
        Display[] displays = mDisplayManager.getPresentationDisplays();

        // If there was available displays but right now no one is available for presentation,
        // we need to notify the display availability changes and reset the display count.
        if (displays.length == 0 && mAvailableDisplayCount > 0) {
            notifyAvailabilityChanged(false);
            mAvailableDisplayCount = 0;
            closePresentationContent();
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
        Activity activity = mActivity.get();
        if (activity == null) return;
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
            dismissPresentationView();
        }

        // If the presentation view is not NULL and its associated display is not changed,
        // the displaying system will automatically restore the content on the view once
        // the Activity gets resumed.
        if (mPresentationView == null && mPresentationContent != null) {
            // Remove the content view from its previous view hierarchy if have.
            ViewGroup parent = (ViewGroup)mPresentationContent.getContentView().getParent();
            if (parent != null) parent.removeView(mPresentationContent.getContentView());

            mPresentationView = PresentationView.createInstance(activity, preferredDisplay);
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

    private void dismissPresentationView() {
        if (mPresentationView == null) return;

        mPresentationView.dismiss();
        mPresentationView = null;
    }

    private void closePresentationContent() {
        if (mPresentationContent == null) return;

        mPresentationContent.close();
        mPresentationContent = null;
    }

    @Override
    public void onActivityStateChange(Activity activity, int newState) {
        switch (newState) {
            case ActivityState.RESUMED:
                onResume();
                break;
            case ActivityState.PAUSED:
                dismissPresentationView();
                if (mPresentationContent != null) mPresentationContent.onPause();
                // No need to listen display changes when the activity is paused.
                mDisplayManager.unregisterDisplayListener(mDisplayListener);
                break;
            case ActivityState.DESTROYED:
                // close the presentation content if have.
                closePresentationContent();
                break;
            default:
                break;
        }
    }
}
