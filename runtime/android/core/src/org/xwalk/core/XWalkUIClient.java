// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.net.Uri;
import android.webkit.ValueCallback;

/**
 * This class notifies the embedder UI events/callbacks.
 */
public class XWalkUIClient {
    /**
     * Request display and focus for this XWalkView.
     * @param view the owner XWalkView instance.
     */
    public void onRequestFocus(XWalkView view) {
    }

    /**
     * Notify the client to close the given XWalkView.
     * @param view the owner XWalkView instance.
     */
    public void onJavascriptCloseWindow(XWalkView view) {
    }

    /**
     * The type of JavaScript modal dialog.
     */
    public enum JavascriptMessageType {
        JAVASCRIPT_ALERT,
        JAVASCRIPT_CONFIRM,
        JAVASCRIPT_PROMPT,
        JAVASCRIPT_BEFOREUNLOAD
    }

    /**
     * Tell the client to display a prompt dialog to the user.
     * @param view the owner XWalkView instance.
     * @param type the type of JavaScript modal dialog.
     * @param url the url of the web page which wants to show this dialog.
     * @param message the message to be shown.
     * @param defaultValue the default value string. Only valid for Prompt dialog.
     * @param result the callback to handle the result from caller.
     */
    public boolean onJavascriptModalDialog(XWalkView view, JavascriptMessageType type, String url,
            String message, String defaultValue, XWalkJavascriptResult result) {
        return false;
    }

    /**
     * Tell the client to toggle fullscreen mode.
     * @param view the owner XWalkView instance.
     * @param enterFullscreen true if it has entered fullscreen mode.
     */
    public void onFullscreenToggled(XWalkView view, boolean enterFullscreen) {
    }

    /**
     * Tell the client to open a file chooser.
     * @param view the owner XWalkView instance.
     * @param uploadFile the callback class to handle the result from caller.
     */
    public void openFileChooser(XWalkView view, ValueCallback<Uri> uploadFile,
            String acceptType, String capture) {
        uploadFile.onReceiveValue(null);
    }

    /**
     * Notify the client that the scale applied to the XWalkView has changed.
     * @param view the owner XWalkView instance.
     * @param oldScale the old scale before scaling.
     * @param newScale the current scale factor after scaling.
     */
    public void onScaleChanged(XWalkView view, float oldScale, float newScale) {
    }
}
