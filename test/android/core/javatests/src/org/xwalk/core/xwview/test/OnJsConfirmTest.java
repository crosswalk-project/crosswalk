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
 * Test suite for onJsConfirm().
 */
public class OnJsConfirmTest extends XWalkViewTestBase {
    private TestHelperBridge.OnJsConfirmHelper mOnJsConfirmHelper;
    
    @Override
    public void setUp() throws Exception {
        super.setUp();
        
        mOnJsConfirmHelper = mTestHelperBridge.getOnJsConfirmHelper();
    }

    @SmallTest
    @Feature({"onJsConfirm"})
    public void testOnJsConfirm() throws Throwable {
        String fileContent = getFileContent("js_modal_dialog.html");
        int count = mOnJsConfirmHelper.getCallCount();

        loadDataSync(null, fileContent, "text/html", false);
        clickOnElementId("js_confirm", null);
        mOnJsConfirmHelper.waitForCallback(count);
        assertEquals(1, mOnJsConfirmHelper.getCallCount());
        assertEquals("confirm", mOnJsConfirmHelper.getMessage());
    }

}
