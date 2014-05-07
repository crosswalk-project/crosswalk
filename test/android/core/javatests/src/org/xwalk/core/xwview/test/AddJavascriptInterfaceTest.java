// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

import org.xwalk.core.JavascriptInterface;

/**
 * Test suite for addJavascriptInterface().
 */
public class AddJavascriptInterfaceTest extends XWalkViewTestBase {
    final String mExpectedStr = "xwalk";

    @Override
    public void setUp() throws Exception {
        super.setUp();

        setXWalkClient(new XWalkViewTestBase.TestXWalkClient());
    }

    class TestJavascriptInterface {
        public String getTextWithoutAnnotation() {
            return mExpectedStr;
        }

        @JavascriptInterface
        public String getText() {
            return mExpectedStr;
        }
    }

    private void addJavascriptInterface() {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getXWalkView().addJavascriptInterface(new TestJavascriptInterface(),
                        "testInterface");
            }
        });
    }

    private void raisesExceptionAndSetTitle(String script) throws Throwable {
        executeJavaScriptAndWaitForResult("try { var title = " +
                                          script + ";" +
                                          "  document.title = title;" +
                                          "} catch (exception) {" +
                                          "  document.title = \"xwalk\";" +
                                          "}");
    }

    @SmallTest
    @Feature({"AddJavascriptInterface"})
    public void testAddJavascriptInterface() throws Throwable {
        final String name = "add_js_interface.html";

        addJavascriptInterface();
        loadAssetFile(name);
        assertEquals(mExpectedStr, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"AddJavascriptInterface"})
    public void testAddJavascriptInterfaceWithAnnotation() throws Throwable {
        final String name = "index.html";
        final String xwalkStr = "\"xwalk\"";
        String result;

        addJavascriptInterface();
        loadAssetFile(name);

        result = executeJavaScriptAndWaitForResult("testInterface.getText()");
        assertEquals(xwalkStr, result);

        raisesExceptionAndSetTitle("testInterface.getTextWithoutAnnotation()");

        String title = getTitleOnUiThread();
        assertEquals(mExpectedStr, title);
    }
}
