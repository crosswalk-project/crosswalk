// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.test.util;

import android.app.Activity;
import android.app.Instrumentation;
import android.content.Context;

import java.io.InputStream;
import java.io.IOException;
import java.util.concurrent.TimeUnit;

import org.chromium.content.browser.test.util.CallbackHelper;
import org.xwalk.app.runtime.XWalkRuntimeClient;

public class XWalkRuntimeClientTestUtilBase extends XWalkTestUtilBase<XWalkRuntimeClient> {
    public class PageStatusCallback {
        public void onPageStarted(String url) {
            mCallbackHelpers.onPageStarted(url);
        }

        public void onPageFinished(String url) {
            mCallbackHelpers.didFinishLoad(url);
        }

        public void onReceivedError(int errorCode, String description, String failingUrl) {
            mCallbackHelpers.onReceivedError(errorCode, description, failingUrl);
        }

        public void onReceivedTitle(String title) {
            mCallbackHelpers.onTitleUpdated(title);
        }
    }

    public XWalkRuntimeClientTestUtilBase(XWalkRuntimeClient runtimeView,
            Instrumentation instrumentation) {
        super(runtimeView, instrumentation);
    }

    @Override
    public void loadUrlAsync(final String url) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getTestedView().loadAppFromUrl(url);
            }
        });
    }

    @Override
    public void loadDataAsync(final String data, final String mimeType,
             final boolean isBase64Encoded) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getTestedView().loadDataForTest(data, mimeType, isBase64Encoded);
            }
        });
    }

    public void loadManifestSync(String url) throws Exception {
        CallbackHelper pageFinishedHelper = mCallbackHelpers.getOnPageFinishedHelper();
        int currentCallCount = pageFinishedHelper.getCallCount();
        loadManifestAsync(url);

        pageFinishedHelper.waitForCallback(currentCallCount, 1, WAIT_TIMEOUT_SECONDS,
                TimeUnit.SECONDS);
    }

    public void loadManifestAsync(final String url) throws Exception {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getTestedView().loadAppFromManifest(url);
            }
        });
    }

    public void onPause() throws Exception {
        mInstrumentation.runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getTestedView().onPause();
            }
        });
    }

    public void onResume() throws Exception {
        mInstrumentation.runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getTestedView().onResume();
            }
        });
    }
}
