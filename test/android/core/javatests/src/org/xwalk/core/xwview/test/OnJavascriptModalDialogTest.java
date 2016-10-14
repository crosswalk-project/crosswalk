// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

/**
 * Test suite for OnJavascriptModalDialog(), OnJsAlert(), OnJsConfirm(), OnJsPrompt().
 */
public class OnJavascriptModalDialogTest extends XWalkViewTestBase {
    private TestHelperBridge.OnJavascriptModalDialogHelper mOnJavascriptModalDialogHelper;
    private TestHelperBridge.OnJsAlertHelper mOnJsAlertHelper;
    private TestHelperBridge.OnJsConfirmHelper mOnJsConfirmHelper;
    private TestHelperBridge.OnJsPromptHelper mOnJsPromptHelper;

    @Override
    public void setUp() throws Exception {
        super.setUp();
    
        mOnJavascriptModalDialogHelper = mTestHelperBridge.getOnJavascriptModalDialogHelper();
        mOnJsAlertHelper = mTestHelperBridge.getOnJsAlertHelper();
        mOnJsConfirmHelper = mTestHelperBridge.getOnJsConfirmHelper();
        mOnJsPromptHelper = mTestHelperBridge.getOnJsPromptHelper();
    }

    @SmallTest
    @Feature({"OnJavascriptModalDialog"})
    public void testOnJavascriptModalDialog() throws Throwable {
        final String url = "js_modal_dialog.html";
        String fileContent = getFileContent(url);
        int count = mOnJavascriptModalDialogHelper.getCallCount();

        loadDataSync(fileContent, "text/html", false);
        clickOnElementId("js_modal_dialog", null);
        mOnJavascriptModalDialogHelper.waitForCallback(count);
        assertEquals("hello", mOnJavascriptModalDialogHelper.getMessage());
    }
    
    @SmallTest
    @Feature({"onJsAlert"})
    public void testOnJsAlert() throws Throwable {
        String fileContent = getFileContent("js_modal_dialog.html");
        int count = mOnJsAlertHelper.getCallCount();

        loadDataSync(fileContent, "text/html", false);
        clickOnElementId("js_alert", null);
        mOnJsAlertHelper.waitForCallback(count);
        assertEquals(1, mOnJsAlertHelper.getCallCount());
        assertEquals("alert", mOnJsAlertHelper.getMessage());
    }
    
    @SmallTest
    @Feature({"onJsConfirm"})
    public void testOnJsConfirm() throws Throwable {
        String fileContent = getFileContent("js_modal_dialog.html");
        int count = mOnJsConfirmHelper.getCallCount();

        loadDataSync(fileContent, "text/html", false);
        clickOnElementId("js_confirm", null);
        mOnJsConfirmHelper.waitForCallback(count);
        assertEquals(1, mOnJsConfirmHelper.getCallCount());
        assertEquals("confirm", mOnJsConfirmHelper.getMessage());
    }
    
    @SmallTest
    @Feature({"onJsPrompt"})
    public void testOnJsPrompt() throws Throwable {
        String fileContent = getFileContent("js_modal_dialog.html");
        int count = mOnJsPromptHelper.getCallCount();

        loadDataSync(fileContent, "text/html", false);
        clickOnElementId("js_prompt", null);
        mOnJsPromptHelper.waitForCallback(count);
        assertEquals(1, mOnJsPromptHelper.getCallCount());
        assertEquals("prompt", mOnJsPromptHelper.getMessage());
    }
}
