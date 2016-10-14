// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

/**
 * Test suite for SetUserAgentString().
 */
public class SetUserAgentStringTest extends XWalkViewTestBase {
    private static final String EMPTY_PAGE =
            "<!doctype html>" +
            "<title>Set User Agent String Test</title><p>Set User Agent String Test.</p>";
    private static final String USER_AGENT =
            "Set User Agent String Test Mozilla/5.0 Apple Webkit Cosswalk Mobile Safari";
    private static final String EXPECTED_USER_AGENT =
            "\"Set User Agent String Test Mozilla/5.0 Apple Webkit Cosswalk Mobile Safari\"";

    @Override
    public void setUp() throws Exception {
        super.setUp();

        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                getXWalkView().setUserAgentString(USER_AGENT);
            }
        });
    }

    @SmallTest
    public void testSetUserAgentString() throws Throwable {
        loadDataSync(EMPTY_PAGE, "text/html", false);
        String result = executeJavaScriptAndWaitForResult("navigator.userAgent;");
        assertEquals(EXPECTED_USER_AGENT, result);
    }
}
