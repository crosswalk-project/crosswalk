// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

/**
 * Test suite for evaluateJavascript().
 */
public class EvaluateJavascriptTest extends XWalkViewTestBase {
    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @SmallTest
    @Feature({"EvaluateJavascript"})
    public void testEvaluateJavascript() throws Throwable {
        final String expectedTitle = "xwalk";
        final String name = "index.html";
        final String code = "document.title=\"xwalk\"";

        loadAssetFile(name);
        executeJavaScriptAndWaitForResult(code);
        assertEquals(expectedTitle, getTitleOnUiThread());
    }
}
