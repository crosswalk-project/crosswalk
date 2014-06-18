// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extension.api.screenorientation;

import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.util.Log;

import org.json.JSONObject;
import org.xwalk.core.extension.XWalkExtension;
import org.xwalk.core.extension.XWalkExtensionContext;

/**
 * XWalk extension for screen orientation api implementation on Android.
 */
public class ScreenOrientationExtension extends XWalkExtension {
    public final static String TAG = "ScreenOrientationExtension";
    public final static String NAME = "xwalk.screen";
    public final static String JS_API_PATH = "jsapi/screen_orientation_api.js";
    public final static String JS_VALUE_TYPE = "value";
    public final static String[] JS_ENTRY_POINTS = {
        "window.screen.lockOrientation",
        "window.screen.unlockOrientation"
    };
    public final static int PORTRAIT_PRIMARY = 1 << 0;
    public final static int LANDSCAPE_PRIMARY = 1 << 1;
    public final static int PORTRAIT_SECONDARY  = 1 << 2;
    public final static int LANDSCAPE_SECONDARY = 1 << 3;
    public final static int PORTRAIT = PORTRAIT_PRIMARY | PORTRAIT_SECONDARY;
    public final static int LANDSCAPE = LANDSCAPE_PRIMARY | LANDSCAPE_SECONDARY;
    public final static int ANY = PORTRAIT | LANDSCAPE;
    public final static int UA_DEFAULTS = 0;

    private String getValueString(String message, String type) {
        if (message.isEmpty() || type.isEmpty()) {
            return "";
        }

        try {
            return new JSONObject(message).getString(type);
        } catch (Exception e) {
            e.printStackTrace();
            return "";
        }
    }

    public static String getInsertedString() {
        String insertedString = "var isAndroid = true;\n";
        insertedString += "var uaDefault = ";
        insertedString += ANY;
        insertedString += ";\n";

        return insertedString;
    }

    public ScreenOrientationExtension(String name, String jsApi, String[] entryPoints, XWalkExtensionContext context) {
        super(name, jsApi, entryPoints, context);
    }

    @Override
    public void onMessage(int instanceId, String message) {
        String value = getValueString(message, JS_VALUE_TYPE);
        if (value.isEmpty())
            return;

        int orientation;
        try {
            orientation = Integer.valueOf(value);
        } catch (Exception e) {
            e.printStackTrace();
            return;
        }

        int screen_orientation_value = ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED;
        switch (orientation) {
            case ANY:
            case UA_DEFAULTS: {
               screen_orientation_value = ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED;
               break;
            }
            case LANDSCAPE_PRIMARY: {
                screen_orientation_value = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE;
                break;
            }
            case PORTRAIT_PRIMARY: {
                screen_orientation_value = ActivityInfo.SCREEN_ORIENTATION_PORTRAIT;
                break;
            }
            case LANDSCAPE_SECONDARY: {
                screen_orientation_value = ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE;
                break;
            }
            case PORTRAIT_SECONDARY: {
                screen_orientation_value = ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT;
                break;
            }
            case LANDSCAPE: {
                screen_orientation_value = ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE;
                break;
            }
            case PORTRAIT: {
                screen_orientation_value = ActivityInfo.SCREEN_ORIENTATION_SENSOR_PORTRAIT;
                break;
            }
            default:
                Log.e(TAG, "Invalid orientation value.");
                return;
        }
        mExtensionContext.getActivity().setRequestedOrientation(screen_orientation_value);
    }

    @Override
    public String onSyncMessage(int instanceId, String message) {
        Log.e(TAG, "Unexpected sync message received: " + message);
        return "";
    }
}
