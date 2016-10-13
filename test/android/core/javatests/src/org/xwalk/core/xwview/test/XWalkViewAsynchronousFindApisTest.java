// Copyright 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;
/**
 * Test suite for asynchronous find APIs
 */
public class XWalkViewAsynchronousFindApisTest extends XWalkViewFindApisTestBase {
    final String WOODCHUCK =
          "How much WOOD would a woodchuck chuck if a woodchuck could chuck wOoD?";

    final String data = "<html><head></head><body>" + WOODCHUCK + "</body></html>";

    @SmallTest
    @Feature({"XWalkView", "FindInPage"})
    public void testFindAllFinds() throws Throwable {
        loadDataSync(data, "text/html", false);
        assertEquals(4, findAllAsyncOnUiThread("wood"));
    }

    @SmallTest
    @Feature({"XWalkView", "FindInPage"})
    public void testFindAllDouble() throws Throwable {
        loadDataSync(data, "text/html", false);
        findAllAsyncOnUiThread("wood");
        assertEquals(4, findAllAsyncOnUiThread("chuck"));
    }

    @SmallTest
    @Feature({"XWalkView", "FindInPage"})
    public void testFindAllDoubleNext() throws Throwable {
        loadDataSync(data, "text/html", false);
        assertEquals(4, findAllAsyncOnUiThread("wood"));
        assertEquals(4, findAllAsyncOnUiThread("wood"));
        assertEquals(2, findNextOnUiThread(true));
    }

    @SmallTest
    @Feature({"XWalkView", "FindInPage"})
    public void testFindAllDoesNotFind() throws Throwable {
        loadDataSync(data, "text/html", false);
        assertEquals(0, findAllAsyncOnUiThread("foo"));
    }

    @SmallTest
    @Feature({"XWalkView", "FindInPage"})
    public void testFindAllEmptyPage() throws Throwable {
        loadDataSync(data, "text/html", false);
        assertEquals(0, findAllAsyncOnUiThread("foo"));
    }

    @SmallTest
    @Feature({"XWalkView", "FindInPage"})
    public void testFindAllEmptyString() throws Throwable {
        loadDataSync(data, "text/html", false);
        assertEquals(0, findAllAsyncOnUiThread(""));
    }

    @SmallTest
    @Feature({"XWalkView", "FindInPage"})
    public void testFindNextForward() throws Throwable {
        loadDataSync(data, "text/html", false);
        assertEquals(4, findAllAsyncOnUiThread("wood"));

        for (int i = 2; i <= 4; i++) {
            assertEquals(i - 1, findNextOnUiThread(true));
        }
        assertEquals(0, findNextOnUiThread(true));
    }

    @SmallTest
    @Feature({"XWalkView", "FindInPage"})
    public void testFindNextBackward() throws Throwable {
        loadDataSync(data, "text/html", false);
        assertEquals(4, findAllAsyncOnUiThread("wood"));

        for (int i = 4; i >= 1; i--) {
            assertEquals(i - 1, findNextOnUiThread(false));
        }
        assertEquals(3, findNextOnUiThread(false));
    }

    @SmallTest
    @Feature({"XWalkView", "FindInPage"})
    public void testFindNextBig() throws Throwable {
        loadDataSync(data, "text/html", false);
        assertEquals(4, findAllAsyncOnUiThread("wood"));

        assertEquals(1, findNextOnUiThread(true));
        assertEquals(0, findNextOnUiThread(false));
        assertEquals(3, findNextOnUiThread(false));
        for (int i = 1; i <= 4; i++) {
            assertEquals(i - 1, findNextOnUiThread(true));
        }
        assertEquals(0, findNextOnUiThread(true));
    }

    @SmallTest
    @Feature({"XWalkView", "FindInPage"})
    public void testFindAllEmptyNext() throws Throwable {
        loadDataSync(data, "text/html", false);
        assertEquals(4, findAllAsyncOnUiThread("wood"));
        assertEquals(1, findNextOnUiThread(true));
        assertEquals(0, findAllAsyncOnUiThread(""));
        assertEquals(0, findNextOnUiThread(true));
        assertEquals(0, findAllAsyncOnUiThread(""));
        assertEquals(4, findAllAsyncOnUiThread("wood"));
        assertEquals(1, findNextOnUiThread(true));
    }

    @SmallTest
    @Feature({"XWalkView", "FindInPage"})
    public void testClearMatches() throws Throwable {
        loadDataSync(data, "text/html", false);
        assertEquals(4, findAllAsyncOnUiThread("wood"));
        clearMatchesOnUiThread();
    }

    @SmallTest
    @Feature({"XWalkView", "FindInPage"})
    public void testClearFindNext() throws Throwable {
        loadDataSync(data, "text/html", false);
        assertEquals(4, findAllAsyncOnUiThread("wood"));
        clearMatchesOnUiThread();
        assertEquals(4, findAllAsyncOnUiThread("wood"));
        assertEquals(1, findNextOnUiThread(true));
    }

    @SmallTest
    @Feature({"XWalkView", "FindInPage"})
    public void testFindEmptyNext() throws Throwable {
        loadDataSync(data, "text/html", false);
        assertEquals(0, findAllAsyncOnUiThread(""));
        assertEquals(0, findNextOnUiThread(true));
        assertEquals(4, findAllAsyncOnUiThread("wood"));
    }

    @SmallTest
    @Feature({"XWalkView", "FindInPage"})
    public void testFindNextFirst() throws Throwable {
        loadDataSync(data, "text/html", false);
        runTestOnUiThread(new Runnable() {
            @Override
            public void run() {
                getXWalkView().findNext(true);
            }
        });
        assertEquals(4, findAllAsyncOnUiThread("wood"));
        assertEquals(1, findNextOnUiThread(true));
        assertEquals(0, findNextOnUiThread(false));
        assertEquals(3, findNextOnUiThread(false));
    }
}
