// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;
import org.chromium.content.browser.test.util.CallbackHelper;

public class ClearClientCertPreferenceTest extends XWalkViewTestBase {
    
    private static class ClearClientCertCallbackHelper extends CallbackHelper
            implements Runnable {
        @Override
        public void run() {
            notifyCalled();
        }
    }
    
    @Feature({"ClearClientCertPreference"})
    @SmallTest
    public void testClearClientCertPreference() throws Throwable {
        final ClearClientCertCallbackHelper callbackHelper = new ClearClientCertCallbackHelper();
        int currentCallCount = callbackHelper.getCallCount();
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                // Make sure calling clearClientCertPreferences with null callback does not
                // cause a crash.
                getXWalkView().clearClientCertPreferences(null);
                getXWalkView().clearClientCertPreferences(callbackHelper);
            }
        });
        callbackHelper.waitForCallback(currentCallCount);
    }
}
