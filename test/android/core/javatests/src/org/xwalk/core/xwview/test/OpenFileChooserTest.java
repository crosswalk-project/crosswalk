// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

/**
 * Test suite for OpenFileChooser().
 */
public class OpenFileChooserTest extends XWalkViewTestBase {
    private TestHelperBridge.OpenFileChooserHelper mOpenFileChooserHelper;

    @Override
    public void setUp() throws Exception {
        super.setUp();
        mOpenFileChooserHelper = mTestHelperBridge.getOpenFileChooserHelper();
    }

    @SmallTest
    @Feature({"OpenFileChooser"})
    public void testOpenFileChooser() throws Throwable {
        final String name = "file_chooser.html";
        String fileContent = getFileContent(name);
        int count = mOpenFileChooserHelper.getCallCount();

        loadDataSync(null, fileContent, "text/html", false);
        clickOnElementId("upload_input", null);
        mOpenFileChooserHelper.waitForCallback(count);
        assertNotNull(mOpenFileChooserHelper.getCallback());
    }
}
