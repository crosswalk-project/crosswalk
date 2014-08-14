// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

/**
 * Test suite for OnJavascriptModalDialog().
 */
public class OnJavascriptModalDialogTest extends XWalkViewTestBase {
    private TestHelperBridge.OnJavascriptModalDialogHelper mOnJavascriptModalDialogHelper;

    @Override
    public void setUp() throws Exception {
        super.setUp();
        mOnJavascriptModalDialogHelper = mTestHelperBridge.getOnJavascriptModalDialogHelper();
    }

    @SmallTest
    @Feature({"OnJavascriptModalDialog"})
    public void testOnJavascriptModalDialog() throws Throwable {
        final String url = "js_modal_dialog.html";
        String fileContent = getFileContent(url);
        int count = mOnJavascriptModalDialogHelper.getCallCount();

        loadDataSync(null, fileContent, "text/html", false);
        clickOnElementId("js_modal_dialog", null);
        mOnJavascriptModalDialogHelper.waitForCallback(count);
        assertEquals("hello", mOnJavascriptModalDialogHelper.getMessage());
    }
}
