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
 * Test suite for onJsPrompt().
 */
public class OnJsPromptTest extends XWalkViewTestBase {
    private TestHelperBridge.OnJsPromptHelper mOnJsPromptHelper;
    
    @Override
    public void setUp() throws Exception {
        super.setUp();

        mOnJsPromptHelper = mTestHelperBridge.getOnJsPromptHelper();
    }

    @SmallTest
    @Feature({"onJsPrompt"})
    public void testOnJsPrompt() throws Throwable {
        String fileContent = getFileContent("js_modal_dialog.html");
        int count = mOnJsPromptHelper.getCallCount();

        loadDataSync(null, fileContent, "text/html", false);
        clickOnElementId("js_prompt", null);
        mOnJsPromptHelper.waitForCallback(count);
        assertEquals(1, mOnJsPromptHelper.getCallCount());
        assertEquals("prompt", mOnJsPromptHelper.getMessage());
    }

}
