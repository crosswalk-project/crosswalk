// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

import org.xwalk.core.XWalkPreferences;

import java.util.concurrent.Callable;

/**
 * Test suite for setValue(), getValue().
 */
public class SetAndGetVauleForPreferencesTest extends XWalkViewTestBase {

    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    private void setValue(final String key, final boolean enabled) {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                try {
                    XWalkPreferences.setValue(key, enabled);
                } catch (Exception e) {
                    assertTrue(e instanceof RuntimeException);
                }
            }
        });
    }

    private boolean getValue(final String key) throws Throwable {
        return runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() {
                try {
                    return XWalkPreferences.getValue(key);
                } catch (Exception e) {
                    assertTrue(e instanceof RuntimeException);
                    return false;
                }
            }
        });
    }

    @SmallTest
    @Feature({"setValue,getValue"})
    public void testSetAndGetValue() throws Throwable {
        boolean result;

        // remote debugging.
        setValue(XWalkPreferences.REMOTE_DEBUGGING, true);
        result = getValue(XWalkPreferences.REMOTE_DEBUGGING);
        assertTrue(result);

        setValue(XWalkPreferences.REMOTE_DEBUGGING, false);
        result = getValue(XWalkPreferences.REMOTE_DEBUGGING);
        assertFalse(result);

        // invalid value.
        String key = "invalid-value";
        setValue(key, true);
        result = getValue(key);
        assertFalse(result);

        setValue(key, false);
        result = getValue(key);
        assertFalse(result);
    }
}
