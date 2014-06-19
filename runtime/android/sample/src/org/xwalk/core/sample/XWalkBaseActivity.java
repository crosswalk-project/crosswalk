// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import org.xwalk.core.XWalkJavascriptResult;
import org.xwalk.core.XWalkUIClient;
import org.xwalk.core.XWalkView;

import android.app.Activity;
import android.net.Uri;
import android.webkit.ValueCallback;
import android.widget.TextView;

public class XWalkBaseActivity extends Activity {
    protected XWalkView mXWalkView;
    private TextView mTextView = null;
    private TestFrom mFrom;
    enum TestFrom {
        NONE,
        CHOOSER,
        SCALE,
        FOCUS,
        TOGGLED,
        DIALOG,
    }
    /*
     * When the activity is paused, XWalkView.onHide() and XWalkView.pauseTimers() need to be called.
     */
    @Override
    public void onPause() {
        super.onPause();
        if (mXWalkView != null) {
            mXWalkView.onHide();
            mXWalkView.pauseTimers();
        }
    }

    /*
     * When the activity is resumed, XWalkView.onShow() and XWalkView.resumeTimers() need to be called.
     */
    @Override
    public void onResume() {
        super.onResume();
        if (mXWalkView != null) {
            mXWalkView.onShow();
            mXWalkView.resumeTimers();
        }
    }

    /*
     * Call onDestroy on XWalkView to release native resources when the activity is destroyed.
     */
    @Override
    public void onDestroy() {
        super.onDestroy();
        if (mXWalkView != null) {
            mXWalkView.onDestroy();
        }
    }

    public void setText(String text) {
        if (mTextView != null) {
            mTextView.setText(text);
        }
    }

    class UIClientBase extends XWalkUIClient {
        public UIClientBase(XWalkView xwalkView, TextView textView, TestFrom from) {
            super(xwalkView);
            mTextView = textView;
            mFrom = from;
        }

        public void onJavascriptCloseWindow(XWalkView view) {
            super.onJavascriptCloseWindow(view);
        }

        public boolean onJavascriptModalDialog(XWalkView view, JavascriptMessageType type,
                String url, String message, String defaultValue, XWalkJavascriptResult result) {
            if (mFrom == TestFrom.DIALOG) {
                setText("API: onJavascriptModalDialog works fine.");
            }
            return super.onJavascriptModalDialog(view, type, url, message, defaultValue, result);
        }

        public void onFullscreenToggled(XWalkView view, boolean enterFullscreen) {
            super.onFullscreenToggled(view, enterFullscreen);
            String str;
            if (mFrom == TestFrom.TOGGLED) {
                if (enterFullscreen) {
                    str = "Entered fullscreen.";
                } else {
                    str = "Exited fullscreen.";
                }
                setText("API: onFullscreenToggled works fine: " + str);
            }
        }

        public void openFileChooser(XWalkView view, ValueCallback<Uri> uploadFile,
                String acceptType, String capture) {
            super.openFileChooser(view, uploadFile, acceptType, capture);
            if (mFrom == TestFrom.CHOOSER) {
                setText("API: openFileChooser works fine.");
            }
        }

        public void onScaleChanged(XWalkView view, float oldScale, float newScale) {
            super.onScaleChanged(view, oldScale, newScale);
            if (mFrom == TestFrom.SCALE) {
                setText("API: onScaleChanged works fine.");
            }
        }

        public void onRequestFocus(XWalkView view) {
            if (mFrom == TestFrom.FOCUS) {
                setText("API: onRequestFocus works fine.");
            }
        }
    }
}
