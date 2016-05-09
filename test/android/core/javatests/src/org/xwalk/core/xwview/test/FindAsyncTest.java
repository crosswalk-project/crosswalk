// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

/**
 * Test suite for asynchronous find APIs
 */
public class FindAsyncTest extends XWalkViewTestBase {
    private TestHelperBridge.OnFindResultReceivedHelper mOnFindResultReceivedHelper;

    @Override
    public void setUp() throws Exception {
        super.setUp();

        mOnFindResultReceivedHelper = mTestHelperBridge.getOnFindResultReceivedHelper();
        setFindListener();
    }

    @SmallTest
    @Feature({"FindAllAsync"})
    public void testOnFindResultReceived() throws Throwable {
        String fileContent = getFileContent("find.html");
        loadDataSync(null, fileContent, "text/html", false);

        int currentCallCount = mOnFindResultReceivedHelper.getCallCount();
        findAllAsync("Find");
        mOnFindResultReceivedHelper.waitForCallback(currentCallCount, 3);
        assertEquals(0, mOnFindResultReceivedHelper.getIndex());
        assertEquals(3, mOnFindResultReceivedHelper.getMatches());
        assertTrue(mOnFindResultReceivedHelper.isDone());

        currentCallCount = mOnFindResultReceivedHelper.getCallCount();
        findNext(true);
        mOnFindResultReceivedHelper.waitForCallback(currentCallCount, 2);
        assertEquals(1, mOnFindResultReceivedHelper.getIndex());
        assertEquals(3, mOnFindResultReceivedHelper.getMatches());
        assertTrue(mOnFindResultReceivedHelper.isDone());

        currentCallCount = mOnFindResultReceivedHelper.getCallCount();
        findNext(true);
        mOnFindResultReceivedHelper.waitForCallback(currentCallCount, 2);
        assertEquals(2, mOnFindResultReceivedHelper.getIndex());
        assertEquals(3, mOnFindResultReceivedHelper.getMatches());
        assertTrue(mOnFindResultReceivedHelper.isDone());

        currentCallCount = mOnFindResultReceivedHelper.getCallCount();
        findNext(false);
        mOnFindResultReceivedHelper.waitForCallback(currentCallCount, 2);
        assertEquals(1, mOnFindResultReceivedHelper.getIndex());
        assertEquals(3, mOnFindResultReceivedHelper.getMatches());
        assertTrue(mOnFindResultReceivedHelper.isDone());
    }
}
