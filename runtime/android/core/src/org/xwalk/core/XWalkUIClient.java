// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.net.Uri;
import android.os.Message;
import android.view.KeyEvent;
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
     * @since 1.0
     */
    public XWalkUIClient(XWalkView view) {
        super(view);
    }

    /**
     * The type of JavaScript modal dialog.
     * @since 1.0
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

    /**
     * Tell the client to display a prompt dialog to the user.
     * @param view the owner XWalkView instance.
     * @param type the type of JavaScript modal dialog.
     * @param url the url of the web page which wants to show this dialog.
     * @param message the message to be shown.
     * @param defaultValue the default value string. Only valid for Prompt dialog.
     * @param result the callback to handle the result from caller.
     * @since 1.0
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

    /**
     * @hide
     */
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
     * Request display and focus for this XWalkView.
     * @param view the owner XWalkView instance.
     * @since 1.0
     */
    public void onRequestFocus(XWalkView view) {
        super.onRequestFocus(view);
    }

    /**
     * @hide
     */
    @Override
    public void onRequestFocus(XWalkViewInternal view) {
        if (view instanceof XWalkView) {
            onRequestFocus((XWalkView) view);
        } else {
            super.onRequestFocus(view);
        }
    }

    /**
     * Notify the client to close the given XWalkView.
     * @param view the owner XWalkView instance.
     * @since 1.0
     */
    public void onJavascriptCloseWindow(XWalkView view) {
        super.onJavascriptCloseWindow(view);
    }

    /**
     * @hide
     */
    @Override
    public void onJavascriptCloseWindow(XWalkViewInternal view) {
        if (view instanceof XWalkView) {
            onJavascriptCloseWindow((XWalkView) view);
        } else {
            super.onJavascriptCloseWindow(view);
        }
    }

    /**
     * Tell the client to toggle fullscreen mode.
     * @param view the owner XWalkView instance.
     * @param enterFullscreen true if it has entered fullscreen mode.
     * @since 1.0
     */
    public void onFullscreenToggled(XWalkView view, boolean enterFullscreen) {
        super.onFullscreenToggled(view, enterFullscreen);
    }

    /**
     * @hide
     */
    @Override
    public void onFullscreenToggled(XWalkViewInternal view, boolean enterFullscreen) {
        if (view instanceof XWalkView) {
            onFullscreenToggled((XWalkView) view, enterFullscreen);
        } else {
            super.onFullscreenToggled(view, enterFullscreen);
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
     * @since 1.0
     */
    public void openFileChooser(XWalkView view, ValueCallback<Uri> uploadFile,
            String acceptType, String capture) {
        super.openFileChooser(view, uploadFile, acceptType, capture);
    }

    /**
     * @hide
     */
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
     * Notify the client that the scale applied to the XWalkView has changed.
     * @param view the owner XWalkView instance.
     * @param oldScale the old scale before scaling.
     * @param newScale the current scale factor after scaling.
     * @since 1.0
     */
    public void onScaleChanged(XWalkView view, float oldScale, float newScale) {
        super.onScaleChanged(view, oldScale, newScale);
    }

    /**
     * @hide
     */
    @Override
    public void onScaleChanged(XWalkViewInternal view, float oldScale, float newScale) {
        if (view instanceof XWalkView) {
            onScaleChanged((XWalkView) view, oldScale, newScale);
        } else {
            super.onScaleChanged(view, oldScale, newScale);
        }
    }

    /**
     * Give the host application a chance to handle the key event synchronously.
     * e.g. menu shortcut key events need to be filtered this way. If return
     * true, XWalkView will not handle the key event. If return false, XWalkView
     * will always handle the key event, so none of the super in the view chain
     * will see the key event. The default behavior returns false.
     *
     * @param view The XWalkView that is initiating the callback.
     * @param event The key event.
     * @return True if the host application wants to handle the key event
     *         itself, otherwise return false
     *
     * @since 2.1
     */
    public boolean shouldOverrideKeyEvent(XWalkView view, KeyEvent event) {
        return super.shouldOverrideKeyEvent(view, event);
    }

    /**
     * @hide
     */
    @Override
    public boolean shouldOverrideKeyEvent(XWalkViewInternal view, KeyEvent event) {
        if (view instanceof XWalkView) {
            return shouldOverrideKeyEvent((XWalkView) view, event);
        }

        return super.shouldOverrideKeyEvent(view, event);
    }

    /**
     * Notify the host application that a key was not handled by the XWalkView.
     * Except system keys, XWalkView always consumes the keys in the normal flow
     * or if shouldOverrideKeyEvent returns true. This is called asynchronously
     * from where the key is dispatched. It gives the host application a chance
     * to handle the unhandled key events.
     *
     * @param view The XWalkView that is initiating the callback.
     * @param event The key event.
     *
     * @since 2.1
     */
    public void onUnhandledKeyEvent(XWalkView view, KeyEvent event) {
        super.onUnhandledKeyEvent(view, event);
    }

    /**
     * @hide
     */
    @Override
    public void onUnhandledKeyEvent(XWalkViewInternal view, KeyEvent event) {
        if (view instanceof XWalkView) {
            onUnhandledKeyEvent((XWalkView) view, event);
        } else {
            super.onUnhandledKeyEvent(view, event);
        }
    }

    /**
     * Notify the host application of a change in the document title.
     * @param view The XWalkView that initiated the callback.
     * @param title A String containing the new title of the document.
     * @since 2.1
     */
    public void onReceivedTitle(XWalkView view, String title) {
        super.onReceivedTitle(view, title);
    }

    @Override
    public void onReceivedTitle(XWalkViewInternal view, String title) {
        if (view instanceof XWalkView) {
            onReceivedTitle((XWalkView) view, title);
        } else {
            super.onReceivedTitle(view, title);
        }
    }

    /**
     * The status when a page stopped loading
     * @since 2.1
     */
    public enum LoadStatus {
        /** Loading finished. */
        FINISHED,
        /** Loading failed. */
        FAILED,
        /** Loading cancelled by user. */
        CANCELLED
    }

    /**
     * Notify the host application that a page has started loading. This method
     * is called once for each main frame load so a page with iframes or
     * framesets will call onPageLoadStarted one time for the main frame. This also
     * means that onPageLoadStarted will not be called when the contents of an
     * embedded frame changes, i.e. clicking a link whose target is an iframe.
     *
     * @param view The XWalkView that is initiating the callback.
     * @param url The url to be loaded.
     *
     * @since 2.1
     */
    public void onPageLoadStarted(XWalkView view, String url) {
        super.onPageLoadStarted(view, url);
    }

    /**
     * @hide
     */
    @Override
    public void onPageLoadStarted(XWalkViewInternal view, String url) {
        if (view instanceof XWalkView) {
            onPageLoadStarted((XWalkView) view, url);
        } else {
            super.onPageLoadStarted(view, url);
        }
    }

    /**
     * Notify the host application that a page has stopped loading. This method
     * is called only for main frame. When onPageLoadStopped() is called, the
     * rendering picture may not be updated yet. To get the notification for the
     * new Picture, use {@link XWalkView.PictureListener#onNewPicture}.
     *
     * @param view The XWalkView that is initiating the callback.
     * @param url The url of the page.
     * @param status the status when the page stopped loading.
     *
     * @since 2.1
     */
    public void onPageLoadStopped(XWalkView view, String url, LoadStatus status) {
        LoadStatusInternal statusInternal = LoadStatusInternal.valueOf(status.toString());
        super.onPageLoadStopped(view, url, statusInternal);
    }

    /**
     * @hide
     */
    @Override
    public void onPageLoadStopped(
            XWalkViewInternal view, String url, LoadStatusInternal statusInternal) {
        LoadStatus status = LoadStatus.valueOf(statusInternal.toString());
        if (view instanceof XWalkView) {
            onPageLoadStopped((XWalkView) view, url, status);
        } else {
            super.onPageLoadStopped(view, url, statusInternal);
        }
    }
}
