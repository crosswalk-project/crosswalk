// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extension.api.launchscreen;

import android.content.Intent;
import android.util.Log;

import org.xwalk.core.XWalkLaunchScreenManager;
import org.xwalk.core.extension.XWalkExtension;
import org.xwalk.core.extension.XWalkExtensionContext;

/**
 * A XWalk extension for LaunchScreen API implementation on Android.
 */
public class LaunchScreenExtension extends XWalkExtension {
    public final static String JS_API_PATH = "jsapi/launch_screen_api.js";
    public final static String NAME = "xwalk.launchscreen";
    public final static String[] JS_ENTRY_POINTS = {
        "window.screen.show"
    };

    // Command messages:
    private final static String CMD_HIDE_LAUNCH_SCREEN = "hideLaunchScreen";

    public LaunchScreenExtension(String name, String jsApi, String[] entryPoints, XWalkExtensionContext context) {
        super(name, jsApi, entryPoints, context);
    }

    @Override
    public void onMessage(int instanceId, String message) {
        if (message.equals(CMD_HIDE_LAUNCH_SCREEN)) {
            hideLaunchScreen();
        }
    }

    private void hideLaunchScreen() {
        // TODO: Considered about the performance of broadcast receiver, it will be a little delayed to hide the
        // launch screen. Better to have an api directly to XWalkLaunchScreenManager.
        // Need to be well designed in the future.
        String filterStr = XWalkLaunchScreenManager.getHideLaunchScreenFilterStr();
        Intent intent = new Intent(filterStr);
        mExtensionContext.getActivity().sendBroadcast(intent);
    }
}
