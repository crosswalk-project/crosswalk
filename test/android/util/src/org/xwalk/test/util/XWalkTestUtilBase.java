// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.test.util;

import android.app.Instrumentation;
import android.content.Context;

import java.io.InputStream;
import java.io.IOException;
import java.util.concurrent.TimeUnit;

import org.chromium.content.browser.test.util.CallbackHelper;

public class XWalkTestUtilBase<T> {
    T mTestedView;
    Instrumentation mInstrumentation;
    public final static int WAIT_TIMEOUT_SECONDS = 15;
    protected XWalkTestUtilCallbacks mCallbackHelpers = new XWalkTestUtilCallbacks();

    public XWalkTestUtilBase(T t, Instrumentation instrumentation) {
        mTestedView = t;
        mInstrumentation = instrumentation;
    }

    public T getTestedView() {
        return mTestedView;
    }

    public Instrumentation getInstrumentation() {
        return mInstrumentation;
    }

    public void loadUrlSync(String url) throws Exception {
        CallbackHelper pageFinishedHelper = mCallbackHelpers.getOnPageFinishedHelper();
        int currentCallCount = pageFinishedHelper.getCallCount();
        loadUrlAsync(url);

        pageFinishedHelper.waitForCallback(currentCallCount, 1, WAIT_TIMEOUT_SECONDS,
                TimeUnit.SECONDS);
    }

    // This should be overridded by inherited classes.
    public void loadUrlAsync(final String url) throws Exception {
    }

    public void loadDataSync(final String data, final String mimeType,
            final boolean isBase64Encoded) throws Exception {
        CallbackHelper pageFinishedHelper = mCallbackHelpers.getOnPageFinishedHelper();
        int currentCallCount = pageFinishedHelper.getCallCount();
        loadDataAsync(data, mimeType, isBase64Encoded);

        pageFinishedHelper.waitForCallback(currentCallCount, 1, WAIT_TIMEOUT_SECONDS,
                TimeUnit.SECONDS);
    }

    // This should be overridded by inherited classes.
    public void loadDataAsync(final String data, final String mimeType,
             final boolean isBase64Encoded) throws Exception {
    }

    public String getFileContent(String fileName) throws Exception {
        Context context = mInstrumentation.getContext();
        InputStream inputStream = context.getAssets().open(fileName);
        int size = inputStream.available();
        byte buffer[] = new byte[size];
        inputStream.read(buffer);
        inputStream.close();

        String fileContent = new String(buffer);
        return fileContent;
    }

    public void loadAssetFile(String fileName) throws Exception {
        String fileContent = getFileContent(fileName);
        loadDataSync(fileContent, "text/html", false);
    }

    public String loadAssetFileAndWaitForTitle(String fileName) throws Exception {
        OnTitleUpdatedHelper helper = mCallbackHelpers.getOnTitleUpdatedHelper();
        int currentCallCount = helper.getCallCount();

        loadAssetFile(fileName);

        helper.waitForCallback(currentCallCount, 1, WAIT_TIMEOUT_SECONDS,
                TimeUnit.SECONDS);
        return helper.getTitle();
    }

    public XWalkTestUtilCallbacks getCallbackHelpers() {
        return mCallbackHelpers;
    }
}
