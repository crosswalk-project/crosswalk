// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

/**
 * Test suite for setAcceptLanuages().
 */
public class SetAcceptLanuagesTest extends XWalkViewTestBase {
    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @SmallTest
    @Feature({"setAcceptLanuages"})
    public void testSetAcceptLanuages() throws Throwable {
        String result;
        final String script = "navigator.languages";
        final String[] languages = {"en;q=0.7", "zh-cn", "da,en-gb;q=0.8,en;q=0.7"};
        final String[] expectedLanguages = {"[\"en;q=0.7\"]", "[\"zh-cn\"]", "[\"da\",\"en-gb;q=0.8\",\"en;q=0.7\"]"};

        result = executeJavaScriptAndWaitForResult(script);
        assertNotNull(result);

        for (int i = 0; i < languages.length; i++) {
            setAcceptLanguages(languages[i]);
            result = executeJavaScriptAndWaitForResult(script);
            assertEquals(expectedLanguages[i], result);
        }
    }
}
