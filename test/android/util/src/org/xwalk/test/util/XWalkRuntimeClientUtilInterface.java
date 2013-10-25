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

public class XWalkRuntimeClientUtilInterface {
    XWalkRuntimeClient mRuntimeView;
    Instrumentation mInstrumentation;
    final TestXWalkRuntimeClientContentsClient mTestContentsClient =
            new TestXWalkRuntimeClientContentsClient();
    protected final static int WAIT_TIMEOUT_SECONDS = 15;

    public class PageStatusCallback {
        public void onPageStarted(String url) {
            mTestContentsClient.onPageStarted(url);
        }

        public void onPageFinished(String url) {
            mTestContentsClient.didFinishLoad(url);
        }

        public void onReceivedError(int errorCode, String description, String failingUrl) {
            mTestContentsClient.onReceivedError(errorCode, description, failingUrl);
        }

        public void onReceivedTitle(String title) {
            mTestContentsClient.onTitleUpdated(title);
        }
    }

    public XWalkRuntimeClientUtilInterface(XWalkRuntimeClient runtimeView, Instrumentation instrumentation) {
        mRuntimeView = runtimeView;
        mInstrumentation = instrumentation;
    }

    public void loadUrlSync(String url) throws Exception {
        CallbackHelper pageFinishedHelper = mTestContentsClient.getOnPageFinishedHelper();
        int currentCallCount = pageFinishedHelper.getCallCount();
        loadUrlAsync(url);

        pageFinishedHelper.waitForCallback(currentCallCount, 1, WAIT_TIMEOUT_SECONDS,
                TimeUnit.SECONDS);
    }

    public void loadUrlAsync(final String url) throws Exception {
        mInstrumentation.runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mRuntimeView.loadAppFromUrl(url);
            }
        });
    }

    public void loadDataSync(final String data, final String mimeType,
            final boolean isBase64Encoded) throws Exception {
        CallbackHelper pageFinishedHelper = mTestContentsClient.getOnPageFinishedHelper();
        int currentCallCount = pageFinishedHelper.getCallCount();
        loadDataAsync(data, mimeType, isBase64Encoded);

        pageFinishedHelper.waitForCallback(currentCallCount, 1, WAIT_TIMEOUT_SECONDS,
                TimeUnit.SECONDS);
    }

    public void loadDataAsync(final String data, final String mimeType,
             final boolean isBase64Encoded) throws Exception {
        mInstrumentation.runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mRuntimeView.loadDataForTest(data, mimeType, isBase64Encoded);
            }
        });
    }

    public String getFileContent(String fileName) {
        try {
            Context context = mInstrumentation.getContext();
            InputStream inputStream = context.getAssets().open(fileName);
            int size = inputStream.available();
            byte buffer[] = new byte[size];
            inputStream.read(buffer);
            inputStream.close();

            String fileContent = new String(buffer);
            return fileContent;
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public void loadAssetFile(String fileName) throws Exception {
        String fileContent = getFileContent(fileName);
        loadDataSync(fileContent, "text/html", false);
    }

    public XWalkRuntimeClient getRuntimeView() {
        return mRuntimeView;
    }

    public TestXWalkRuntimeClientContentsClient getContentsClient() {
        return mTestContentsClient;
    }
}
