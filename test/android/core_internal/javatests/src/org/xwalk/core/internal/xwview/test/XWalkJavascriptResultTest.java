// Copyright 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import java.util.concurrent.atomic.AtomicBoolean;

import org.chromium.base.test.util.Feature;
import org.chromium.content.browser.test.util.CallbackHelper;

import org.xwalk.core.internal.XWalkJavascriptResultInternal;
import org.xwalk.core.internal.XWalkUIClientInternal;
import org.xwalk.core.internal.XWalkUIClientInternal.JavascriptMessageTypeInternal;
import org.xwalk.core.internal.XWalkViewInternal;

public class XWalkJavascriptResultTest extends XWalkViewInternalTestBase {
    private static final String EMPTY_PAGE =
            "<!doctype html>" +
            "<title>Modal Dialog Test</title><p>Testcase.</p>";
    private static final String BEFORE_UNLOAD_URL =
            "<!doctype html>" +
            "<head><script>window.onbeforeunload=function() {" +
            "return 'Are you sure?';" +
            "};</script></head></body>";
    final String ALERT_TEXT = "Hello World!";
    final String PROMPT_TEXT = "How do you like your eggs in the morning?";
    final String PROMPT_DEFAULT = "Scrambled";
    final String PROMPT_RESULT = "I like mine with a kiss";
    final String CONFIRM_TEXT = "Would you like a cookie?";
    final AtomicBoolean callbackCalled = new AtomicBoolean(false);
    final CallbackHelper jsBeforeUnloadHelper = new CallbackHelper();
    boolean flagForConfirmCancelled = false;

    class TestXWalkUIClientForJSResult extends XWalkUIClientInternal {
        public TestXWalkUIClientForJSResult() {
            super(getXWalkView());
        }

        @Override
        public void onPageLoadStarted(XWalkViewInternal view, String url) {
            mTestHelperBridge.onPageStarted(url);
        }

        @Override
        public void onPageLoadStopped(XWalkViewInternal view, String url, LoadStatusInternal status) {
            mTestHelperBridge.onPageFinished(url);
        }

        @Override
        public boolean onJavascriptModalDialog(XWalkViewInternal view, JavascriptMessageTypeInternal type,
                String url, String message, String defaultValue, XWalkJavascriptResultInternal result) {
            switch(type) {
                case JAVASCRIPT_ALERT:
                    callbackCalled.set(true);
                    result.confirm();
                    assertEquals(ALERT_TEXT, message);
                    return false;
                case JAVASCRIPT_CONFIRM:
                    assertEquals(CONFIRM_TEXT, message);
                    if (flagForConfirmCancelled == true) {
                        result.cancel();
                    } else {
                        result.confirm();
                    }
                    callbackCalled.set(true);
                    return false;
                case JAVASCRIPT_PROMPT:
                    assertEquals(PROMPT_TEXT, message);
                    assertEquals(PROMPT_DEFAULT, defaultValue);
                    result.confirmWithResult(PROMPT_RESULT);
                    callbackCalled.set(true);
                    return false;
                case JAVASCRIPT_BEFOREUNLOAD:
                    result.cancel();
                    jsBeforeUnloadHelper.notifyCalled();
                    return false;
                default:
                    break;
            }
            assert(false);
            return false;
        }
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        setUIClient(new TestXWalkUIClientForJSResult());
    }

    @SmallTest
    @Feature({"onJsAlert"})
    public void testOverrideAlertHandling() throws Throwable {
        loadDataSync(EMPTY_PAGE, "text/html", false);
        executeJavaScriptAndWaitForResult("alert('" + ALERT_TEXT + "')");
        assertTrue(callbackCalled.get());
    }

    @SmallTest
    @Feature({"onJsPrompt"})
    public void testOverridePromptHandling() throws Throwable {
        loadDataSync(EMPTY_PAGE, "text/html", false);
        String result = executeJavaScriptAndWaitForResult(
                "prompt('" + PROMPT_TEXT + "','" + PROMPT_DEFAULT + "')");
        assertTrue(callbackCalled.get());
        assertEquals("\"" + PROMPT_RESULT + "\"", result);
    }

    @SmallTest
    @Feature({"onJsConfirm"})
    public void testOverrideConfirmHandlingConfirmed() throws Throwable {
        loadDataSync(EMPTY_PAGE, "text/html", false);
        String result = executeJavaScriptAndWaitForResult(
                "confirm('" + CONFIRM_TEXT + "')");
        assertTrue(callbackCalled.get());
        assertEquals("true", result);
    }

    @SmallTest
    @Feature({"onJsConfirm"})
    public void testOverrideConfirmHandlingCancelled() throws Throwable {
        flagForConfirmCancelled = true;
        loadDataSync(EMPTY_PAGE, "text/html", false);
        String result = executeJavaScriptAndWaitForResult(
                "confirm('" + CONFIRM_TEXT + "')");
        assertTrue(callbackCalled.get());
        assertEquals("false", result);
    }

    @SmallTest
    @Feature({"onJsConfirm(JAVASCRIPT_BEFOREUNLOAD)"})
    public void testOverrideBeforeUnloadHandling() throws Throwable {
        loadDataSync(BEFORE_UNLOAD_URL, "text/html", false);

        int callCount = jsBeforeUnloadHelper.getCallCount();
        loadDataAsync(EMPTY_PAGE, "text/html", false);
        jsBeforeUnloadHelper.waitForCallback(callCount);
    }
}
