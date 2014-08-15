// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import android.os.Bundle;
import android.webkit.ValueCallback;

import org.xwalk.core.JavascriptInterface;
import org.xwalk.core.XWalkView;

public class HasEnteredFullscreenActivity extends XWalkBaseActivity {
    boolean mEntered;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.version_layout);
        mXWalkView = (XWalkView) findViewById(R.id.xwalkview);

        mXWalkView.loadAppFromManifest("file:///android_asset/manifest_fullscreen.json", null);
        addJavascriptInterface();
    }

    void setFullscreenResult(boolean result) {
        mEntered = result;
    }

    class TestForFullscreen {
        @JavascriptInterface
        public boolean hasEnteredFullscreen() {
            mXWalkView.post(new Runnable() {
                @Override
                public void run() {
                    setFullscreenResult(mXWalkView.hasEnteredFullscreen());
                }
            });
            return mEntered;
        }

        @JavascriptInterface
        public boolean leaveFullscreen() {
            mXWalkView.post(new Runnable() {
                @Override
                public void run() {
                    mXWalkView.leaveFullscreen();
                    setFullscreenResult(!mXWalkView.hasEnteredFullscreen());
                }
            });
            return mEntered;
        }
    }

    private void addJavascriptInterface() {
        mXWalkView.addJavascriptInterface(new TestForFullscreen(),
                "fullscreen");
    }

    public void evaluateJavascript(String code) {
        ValueCallback<String> callback =
            new ValueCallback<String>() {
                @Override
                public void onReceiveValue(String jsonResult) {
                    // Nothing.
                }
            };
        mXWalkView.evaluateJavascript(code, callback);
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (mXWalkView.hasEnteredFullscreen()) {
            mXWalkView.leaveFullscreen();
            if (!mXWalkView.hasEnteredFullscreen()) {
                String js = "document.getElementById(\"result\").innerHTML = " +
                        "\"hasEnteredFullscreen and leaveFullscreen works fine.\"";
                evaluateJavascript(js);
            }
        }
    }
}
