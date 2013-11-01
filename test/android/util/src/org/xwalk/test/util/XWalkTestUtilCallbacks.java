// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.test.util;

import org.chromium.content.browser.test.util.TestCallbackHelperContainer.OnPageFinishedHelper;
import org.chromium.content.browser.test.util.TestCallbackHelperContainer.OnPageStartedHelper;
import org.chromium.content.browser.test.util.TestCallbackHelperContainer.OnReceivedErrorHelper;

public class XWalkTestUtilCallbacks {
    final OnPageStartedHelper mOnPageStartedHelper;
    final OnPageFinishedHelper mOnPageFinishedHelper;
    final OnReceivedErrorHelper mOnReceivedErrorHelper;
    final OnTitleUpdatedHelper mOnTitleUpdatedHelper;

    public XWalkTestUtilCallbacks() {
        mOnPageStartedHelper = new OnPageStartedHelper();
        mOnPageFinishedHelper = new OnPageFinishedHelper();
        mOnReceivedErrorHelper = new OnReceivedErrorHelper();
        mOnTitleUpdatedHelper = new OnTitleUpdatedHelper();
    }

    public OnPageStartedHelper getOnPageStartedHelper() {
        return mOnPageStartedHelper;
    }

    public OnPageFinishedHelper getOnPageFinishedHelper() {
        return mOnPageFinishedHelper;
    }

    public OnReceivedErrorHelper getOnReceivedErrorHelper() {
        return mOnReceivedErrorHelper;
    }

    public OnTitleUpdatedHelper getOnTitleUpdatedHelper() {
        return mOnTitleUpdatedHelper;
    }

    public void onPageStarted(String url) {
        mOnPageStartedHelper.notifyCalled(url);
    }

    public void onReceivedError(int errorCode, String description, String failingUrl) {
        mOnReceivedErrorHelper.notifyCalled(errorCode, description, failingUrl);
    }

    public void didFinishLoad(String url) {
        mOnPageFinishedHelper.notifyCalled(url);
    }

    public void onTitleUpdated(String title) {
        mOnTitleUpdatedHelper.notifyCalled(title);
    }
}
