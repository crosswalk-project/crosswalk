// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;

import org.chromium.base.test.util.Feature;
import org.xwalk.core.XWalkUIClient;
import org.xwalk.core.XWalkView;

/**
 * Test suite for onJsAlert().
 */
public class OnJsAlertTest extends XWalkViewTestBase {
    private TestHelperBridge.OnJsAlertHelper mOnJsAlertHelper;
    
    @Override
    public void setUp() throws Exception {
        super.setUp();

        mOnJsAlertHelper = mTestHelperBridge.getOnJsAlertHelper();
    }

    @SmallTest
    @Feature({"onJsAlert"})
    public void testOnJsAlert() throws Throwable {
        String fileContent = getFileContent("js_modal_dialog.html");
        int count = mOnJsAlertHelper.getCallCount();

        loadDataSync(null, fileContent, "text/html", false);
        clickOnElementId("js_alert", null);
        mOnJsAlertHelper.waitForCallback(count);
        assertEquals(1, mOnJsAlertHelper.getCallCount());
        assertEquals("alert", mOnJsAlertHelper.getMessage());
    }

}
