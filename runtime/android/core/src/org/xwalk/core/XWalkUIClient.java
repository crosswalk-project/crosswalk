// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.net.Uri;
import android.webkit.ValueCallback;

import org.xwalk.core.internal.XWalkJavascriptResultInternal;
import org.xwalk.core.internal.XWalkUIClientInternal;
import org.xwalk.core.internal.XWalkViewInternal;

/**
 * This class notifies the embedder UI events/callbacks.
 */
public class XWalkUIClient extends XWalkUIClientInternal {

    /**
     * Constructor.
     * @param view the owner XWalkView instance.
     */
    public XWalkUIClient(XWalkView view) {
        super(view);
    }

    /**
     * The type of JavaScript modal dialog.
     */
    public enum JavascriptMessageType {
        /** JavaScript alert dialog. */
        JAVASCRIPT_ALERT,
        /** JavaScript confirm dialog. */
        JAVASCRIPT_CONFIRM,
        /** JavaScript prompt dialog. */
        JAVASCRIPT_PROMPT,
        /** JavaScript dialog for a window-before-unload notification. */
        JAVASCRIPT_BEFOREUNLOAD
    }

    @Override
    public boolean onJavascriptModalDialog(XWalkViewInternal view,
            JavascriptMessageTypeInternal typeInternal,
            String url, String message, String defaultValue, XWalkJavascriptResultInternal result) {
        JavascriptMessageType type = JavascriptMessageType.valueOf(typeInternal.toString());
        if (view instanceof XWalkView) {
            return onJavascriptModalDialog(
                    (XWalkView) view,
                    type, url, message, defaultValue,
                    new XWalkJavascriptResultHandler(result));
        }

        return super.onJavascriptModalDialog(
                view, typeInternal, url, message, defaultValue, result);
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
    public boolean onJavascriptModalDialog(XWalkView view, JavascriptMessageType type,
            String url, String message, String defaultValue, XWalkJavascriptResult result) {
        XWalkJavascriptResultInternal resultInternal =
                ((XWalkJavascriptResultHandler) result).getInternal();
        JavascriptMessageTypeInternal typeInternal =
                JavascriptMessageTypeInternal.valueOf(type.toString());
        return super.onJavascriptModalDialog(
                view, typeInternal, url, message, defaultValue, resultInternal);
    }

    @Override
    public void onRequestFocus(XWalkViewInternal view) {
        if (view instanceof XWalkView) {
            onRequestFocus((XWalkView) view);
        } else {
            super.onRequestFocus(view);
        }
    }

    /**
     * Request display and focus for this XWalkView.
     * @param view the owner XWalkView instance.
     */
    public void onRequestFocus(XWalkView view) {
        super.onRequestFocus(view);
    }

    @Override
    public void onJavascriptCloseWindow(XWalkViewInternal view) {
        if (view instanceof XWalkView) {
            onJavascriptCloseWindow((XWalkView) view);
        } else {
            super.onJavascriptCloseWindow(view);
        }
    }

    /**
     * Notify the client to close the given XWalkView.
     * @param view the owner XWalkView instance.
     */
    public void onJavascriptCloseWindow(XWalkView view) {
        super.onJavascriptCloseWindow(view);
    }

    @Override
    public void onFullscreenToggled(XWalkViewInternal view, boolean enterFullscreen) {
        if (view instanceof XWalkView) {
            onFullscreenToggled((XWalkView) view, enterFullscreen);
        } else {
            super.onFullscreenToggled(view, enterFullscreen);
        }
    }

    /**
     * Tell the client to toggle fullscreen mode.
     * @param view the owner XWalkView instance.
     * @param enterFullscreen true if it has entered fullscreen mode.
     */
    public void onFullscreenToggled(XWalkView view, boolean enterFullscreen) {
        super.onFullscreenToggled(view, enterFullscreen);
    }

    @Override
    public void openFileChooser(XWalkViewInternal view, ValueCallback<Uri> uploadFile,
            String acceptType, String capture) {
        if (view instanceof XWalkView) {
            openFileChooser((XWalkView) view, uploadFile, acceptType, capture);
        } else {
            super.openFileChooser(view, uploadFile, acceptType, capture);
        }
    }

    /**
     * Tell the client to open a file chooser.
     * @param view the owner XWalkView instance.
     * @param uploadFile the callback class to handle the result from caller. It MUST
     *        be invoked in all cases. Leave it not invoked will block all following
     *        requests to open file chooser.
     * @param acceptType value of the 'accept' attribute of the input tag associated
     *        with this file picker.
     * @param capture value of the 'capture' attribute of the input tag associated
     *        with this file picker
     */
    public void openFileChooser(XWalkView view, ValueCallback<Uri> uploadFile,
            String acceptType, String capture) {
        super.openFileChooser(view, uploadFile, acceptType, capture);
    }

    @Override
    public void onScaleChanged(XWalkViewInternal view, float oldScale, float newScale) {
        if (view instanceof XWalkView) {
            onScaleChanged((XWalkView) view, oldScale, newScale);
        } else {
            super.onScaleChanged(view, oldScale, newScale);
        }
    }

    /**
     * Notify the client that the scale applied to the XWalkView has changed.
     * @param view the owner XWalkView instance.
     * @param oldScale the old scale before scaling.
     * @param newScale the current scale factor after scaling.
     */
    public void onScaleChanged(XWalkView view, float oldScale, float newScale) {
        super.onScaleChanged(view, oldScale, newScale);
    }
}
