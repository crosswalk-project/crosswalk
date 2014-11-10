// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;
import android.webkit.ValueCallback;

import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.Feature;
import org.xwalk.core.XWalkUIClient;
import org.xwalk.core.XWalkView;

/**
 * Test suite for OnCreateWindowRequested().
 */
public class OnCreateWindowRequestedTest extends XWalkViewTestBase {
    private TestHelperBridge.OnCreateWindowRequestedHelper mOnCreateWindowRequestedHelper;

    @Override
    public void setUp() throws Exception {
        super.setUp();

        mOnCreateWindowRequestedHelper = mTestHelperBridge.getOnCreateWindowRequestedHelper();

        setUIClient(new XWalkUIClient(getXWalkView()){
            @Override
            public boolean onCreateWindowRequested(XWalkView view, InitiateBy initiator,
                    ValueCallback<XWalkView> callback) {
                Log.d("XWalkView", "onCreateWindowRequested: " + initiator);
                XWalkView newView = new XWalkView(getActivity(), getActivity());

                callback.onReceiveValue(newView);
                mOnCreateWindowRequestedHelper.notifyCalled(newView);
                return true;
            }

        });
    }

    @DisabledTest
    public void testOnCreateWindowRequested() throws Throwable {
        String fileContent = getFileContent("create_window_1.html");
        int count = mOnCreateWindowRequestedHelper.getCallCount();

        loadDataAsync(null, fileContent, "text/html", false);
        clickOnElementId("new_window", null);
        mOnCreateWindowRequestedHelper.waitForCallback(count);
        assertNotNull(mOnCreateWindowRequestedHelper.getXWalkView());
    }
}
