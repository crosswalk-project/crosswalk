// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.graphics.Bitmap;
import android.os.Message;
import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;

import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.Feature;
import org.xwalk.core.XWalkUIClient;
import org.xwalk.core.XWalkView;

/**
 * Test suite for OnReceivedIcon().
 */
public class OnReceivedIconTest extends XWalkViewTestBase {
    private TestHelperBridge.OnReceivedIconHelper mOnReceivedIconHelper;

    @Override
    public void setUp() throws Exception {
        super.setUp();

        mOnReceivedIconHelper = mTestHelperBridge.getOnReceivedIconHelper();

        setUIClient(new XWalkUIClient(getXWalkView()){
            @Override
            public void onIconAvailable(XWalkView view, String url, Message msg) {
                Log.d("XWalkView", "onIconAvailable");
                msg.sendToTarget();
            }

            @Override
            public void onReceivedIcon(XWalkView view, String url, Bitmap icon) {
                Log.d("XWalkView", "onReceivedIcon");
                mOnReceivedIconHelper.notifyCalled(icon);
            }
        });
    }

    @DisabledTest
    public void testOnReceivedIcon() throws Throwable {
        String fileContent = getFileContent("favicon.html");
        int count = mOnReceivedIconHelper.getCallCount();

        loadDataAsync(null, fileContent, "text/html", false);
        mOnReceivedIconHelper.waitForCallback(count);
        assertNotNull(mOnReceivedIconHelper.getIcon());
    }
}
