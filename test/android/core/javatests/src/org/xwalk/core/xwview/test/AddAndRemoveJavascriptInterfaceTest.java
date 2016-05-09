// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

import org.xwalk.core.JavascriptInterface;
import org.xwalk.core.XWalkView;

/**
 * Test suite for addandremoveJavascriptInterface().
 */
public class AddAndRemoveJavascriptInterfaceTest extends XWalkViewTestBase {
    final String mExpectedStr = "xwalk";
    final String mExpectedStr2 = "xwalk2";
    final String mDefaultTitle = "Add JS Interface";

    @Override
    public void setUp() throws Exception {
        super.setUp();
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

    class TestJavascriptInterface2 {
        @JavascriptInterface
        public String getText() {
            return mExpectedStr2;
        }
    }

    private void addJavascriptInterface(final Object object, final String name) {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getXWalkView().addJavascriptInterface(object, name);
            }
        });
    }

    private void removeJavascriptInterface(final String name) {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getXWalkView().removeJavascriptInterface(name);
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
    @Feature({"AddAndRemoveJavascriptInterface"})
    public void testAddAndRemoveJavascriptInterface() throws Throwable {
        final String name = "add_js_interface.html";
        final String xwalkStr2 = "\"xwalk2\"";
        String result;

        addJavascriptInterface(new TestJavascriptInterface(), "testInterface");
        loadAssetFile(name);
        assertEquals(mExpectedStr, getTitleOnUiThread());

        removeJavascriptInterface("testInterface2");
        assertEquals(mExpectedStr, getTitleOnUiThread());

        addJavascriptInterface(new TestJavascriptInterface2(), "testInterface2");
        reloadSync(XWalkView.RELOAD_NORMAL);
        result = executeJavaScriptAndWaitForResult("testInterface2.getText()");
        assertEquals(xwalkStr2, result);

        removeJavascriptInterface("testInterface2");
        reloadSync(XWalkView.RELOAD_NORMAL);
        result = executeJavaScriptAndWaitForResult("testInterface2.getText()");
        assertEquals("null", result);
        assertEquals(mExpectedStr, getTitleOnUiThread());

        removeJavascriptInterface("testInterface");
        assertEquals(mExpectedStr, getTitleOnUiThread());
        reloadSync(XWalkView.RELOAD_NORMAL);
        assertEquals(mDefaultTitle, getTitleOnUiThread());

        addJavascriptInterface(new TestJavascriptInterface(), "testInterface");
        reloadSync(XWalkView.RELOAD_NORMAL);
        assertEquals(mExpectedStr, getTitleOnUiThread());
    }

    @SmallTest
    @Feature({"AddAndRemoveJavascriptInterface"})
    public void testAddJavascriptInterfaceWithAnnotation() throws Throwable {
        final String name = "index.html";
        final String xwalkStr = "\"xwalk\"";
        String result;

        addJavascriptInterface(new TestJavascriptInterface(), "testInterface");
        loadAssetFile(name);

        result = executeJavaScriptAndWaitForResult("testInterface.getText()");
        assertEquals(xwalkStr, result);

        raisesExceptionAndSetTitle("testInterface.getTextWithoutAnnotation()");

        String title = getTitleOnUiThread();
        assertEquals(mExpectedStr, title);
    }
}
