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
 * Test suite for OnConsoleMessage().
 */
public class OnConsoleMessageTest extends XWalkViewTestBase {
    private TestHelperBridge.OnConsoleMessageHelper mOnConsoleMessageHelper;
    private static final int NUM_OF_CONSOLE_CALL = 10;

    @Override
    public void setUp() throws Exception {
        super.setUp();

        mOnConsoleMessageHelper = mTestHelperBridge.getOnConsoleMessageHelper();
    }

    @SmallTest
    @Feature({"OnConsoleMessage"})
    public void testOnConsoleMessageDebug() throws Throwable {
        String fileContent = getFileContent("console_message.html");
        int count = mOnConsoleMessageHelper.getCallCount();

        loadDataSync(fileContent, "text/html", false);
        executeJavaScriptAndWaitForResult("doDebug();");
        mOnConsoleMessageHelper.waitForCallback(count);
        assertEquals(1, mOnConsoleMessageHelper.getCallCount());
        assertEquals("debug", mOnConsoleMessageHelper.getMessage());
    }

    @SmallTest
    @Feature({"OnConsoleMessage"})
    public void testOnConsoleMessageError() throws Throwable {
        String fileContent = getFileContent("console_message.html");
        int count = mOnConsoleMessageHelper.getCallCount();

        loadDataSync(fileContent, "text/html", false);
        executeJavaScriptAndWaitForResult("doError();");
        mOnConsoleMessageHelper.waitForCallback(count);
        assertEquals(1, mOnConsoleMessageHelper.getCallCount());
        assertEquals("error", mOnConsoleMessageHelper.getMessage());
    }

    @SmallTest
    @Feature({"OnConsoleMessage"})
    public void testOnConsoleMessageInfo() throws Throwable {
        String fileContent = getFileContent("console_message.html");
        int count = mOnConsoleMessageHelper.getCallCount();

        loadDataSync(fileContent, "text/html", false);
        executeJavaScriptAndWaitForResult("doInfo();");
        mOnConsoleMessageHelper.waitForCallback(count);
        assertEquals(1, mOnConsoleMessageHelper.getCallCount());
        assertEquals("info", mOnConsoleMessageHelper.getMessage());
    }

    @SmallTest
    @Feature({"OnConsoleMessage"})
    public void testOnConsoleMessageLog() throws Throwable {
        String fileContent = getFileContent("console_message.html");
        int count = mOnConsoleMessageHelper.getCallCount();

        loadDataSync(fileContent, "text/html", false);
        executeJavaScriptAndWaitForResult("doLog();");
        mOnConsoleMessageHelper.waitForCallback(count);
        assertEquals(1, mOnConsoleMessageHelper.getCallCount());
        assertEquals("log", mOnConsoleMessageHelper.getMessage());
    }

    @SmallTest
    @Feature({"OnConsoleMessage"})
    public void testOnConsoleMessageWarn() throws Throwable {
        String fileContent = getFileContent("console_message.html");
        int count = mOnConsoleMessageHelper.getCallCount();

        loadDataSync(fileContent, "text/html", false);
        executeJavaScriptAndWaitForResult("doWarn();");
        mOnConsoleMessageHelper.waitForCallback(count);
        assertEquals(1, mOnConsoleMessageHelper.getCallCount());
        assertEquals("warn", mOnConsoleMessageHelper.getMessage());
    }

    @SmallTest
    @Feature({"OnConsoleMessage"})
    public void testOnConsoleMessageDir() throws Throwable {
        String fileContent = getFileContent("console_message.html");
        int count = mOnConsoleMessageHelper.getCallCount();

        loadDataSync(fileContent, "text/html", false);
        executeJavaScriptAndWaitForResult("doDir();");
        mOnConsoleMessageHelper.waitForCallback(count);
        assertEquals(1, mOnConsoleMessageHelper.getCallCount());
        assertEquals("dir", mOnConsoleMessageHelper.getMessage());
    }

    @SmallTest
    @Feature({"OnConsoleMessage"})
    public void testOnConsoleMessageDirxml() throws Throwable {
        String fileContent = getFileContent("console_message.html");
        int count = mOnConsoleMessageHelper.getCallCount();

        loadDataSync(fileContent, "text/html", false);
        executeJavaScriptAndWaitForResult("doDirxml();");
        mOnConsoleMessageHelper.waitForCallback(count);
        assertEquals(1, mOnConsoleMessageHelper.getCallCount());
        assertEquals("dirxml", mOnConsoleMessageHelper.getMessage());
    }

    @SmallTest
    @Feature({"OnConsoleMessage"})
    public void testOnConsoleMessageTable() throws Throwable {
        String fileContent = getFileContent("console_message.html");
        int count = mOnConsoleMessageHelper.getCallCount();

        loadDataSync(fileContent, "text/html", false);
        executeJavaScriptAndWaitForResult("doTable();");
        mOnConsoleMessageHelper.waitForCallback(count);
        assertEquals(1, mOnConsoleMessageHelper.getCallCount());
        assertEquals("table", mOnConsoleMessageHelper.getMessage());
    }

    @SmallTest
    @Feature({"OnConsoleMessage"})
    public void testOnConsoleMessageClear() throws Throwable {
        String fileContent = getFileContent("console_message.html");
        int count = mOnConsoleMessageHelper.getCallCount();

        loadDataSync(fileContent, "text/html", false);
        executeJavaScriptAndWaitForResult("doClear();");
        mOnConsoleMessageHelper.waitForCallback(count);
        assertEquals(1, mOnConsoleMessageHelper.getCallCount());
        assertEquals("clear", mOnConsoleMessageHelper.getMessage());
    }

    @SmallTest
    @Feature({"OnConsoleMessage"})
    public void testOnConsoleMessageTrace() throws Throwable {
        String fileContent = getFileContent("console_message.html");
        int count = mOnConsoleMessageHelper.getCallCount();

        loadDataSync(fileContent, "text/html", false);
        executeJavaScriptAndWaitForResult("doTrace();");
        mOnConsoleMessageHelper.waitForCallback(count);
        assertEquals(1, mOnConsoleMessageHelper.getCallCount());
        assertEquals("trace", mOnConsoleMessageHelper.getMessage());
    }

    @SmallTest
    @Feature({"OnConsoleMessage"})
    public void testOnConsoleMessageAll() throws Throwable {
        String fileContent = getFileContent("console_message.html");
        int count = mOnConsoleMessageHelper.getCallCount();

        loadDataSync(fileContent, "text/html", false);
        executeJavaScriptAndWaitForResult("doAll();");
        mOnConsoleMessageHelper.waitForCallback(count,NUM_OF_CONSOLE_CALL);
        assertEquals(NUM_OF_CONSOLE_CALL, mOnConsoleMessageHelper.getCallCount());
    }

}
