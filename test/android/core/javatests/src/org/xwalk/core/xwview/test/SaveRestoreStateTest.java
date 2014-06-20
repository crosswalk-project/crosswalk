// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.app.Activity;
import android.os.Bundle;
import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;

import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.Feature;
import org.chromium.content.browser.ContentViewCore;
import org.chromium.content.browser.NavigationEntry;
import org.chromium.content.browser.NavigationHistory;
import org.chromium.net.test.util.TestWebServer;

import org.xwalk.core.XWalkNavigationHistory;
import org.xwalk.core.XWalkView;
import org.xwalk.core.internal.XWalkClient;
import org.xwalk.core.xwview.test.util.CommonResources;

import java.util.concurrent.Callable;

public class SaveRestoreStateTest extends XWalkViewTestBase {

    private TestWebServer mWebServer;

    private static final int NUM_NAVIGATIONS = 3;
    private static final String TITLES[] = {
            "page 1 title foo",
            "page 2 title bar",
            "page 3 title baz"
    };
    private static final String PATHS[] = {
            "/p1foo.html",
            "/p2bar.html",
            "/p3baz.html",
    };

    private String mUrls[];
    private XWalkView mXWalkView;
    private XWalkView mRestoreXWalkView;

    @Override
    public void setUp() throws Exception {
        super.setUp();

        final Activity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                mXWalkView = getXWalkView();
                mRestoreXWalkView = new XWalkView(activity, activity);
                mXWalkView.setXWalkClient(new XWalkViewTestBase.TestXWalkClient());
            }
        });

        mUrls = new String[NUM_NAVIGATIONS];
        mWebServer = new TestWebServer(false);
    }

    @Override
    public void tearDown() throws Exception {
        if (mWebServer != null) {
            mWebServer.shutdown();
        }
        super.tearDown();
    }

    private void setServerResponseAndLoad(int upto) throws Throwable {
        for (int i = 0; i < upto; ++i) {
            String html = CommonResources.makeHtmlPageFrom(
                    "<title>" + TITLES[i] + "</title>",
                    "");
            mUrls[i] = mWebServer.setResponse(PATHS[i], html, null);
            loadUrlSync(mUrls[i]);
        }
    }

    private XWalkNavigationHistory getNavigationHistoryOnUiThread(
            final XWalkView content) throws Throwable{
        return runTestOnUiThreadAndGetResult(new Callable<XWalkNavigationHistory>() {
            @Override
            public XWalkNavigationHistory call() throws Exception {
                return content.getNavigationHistory();
            }
        });
    }

    private void checkHistoryItemList(XWalkView content) throws Throwable {
        XWalkNavigationHistory history = getNavigationHistoryOnUiThread(content);
        assertEquals(NUM_NAVIGATIONS, history.size());
        assertEquals(NUM_NAVIGATIONS - 1, history.getCurrentIndex());

        // Note this is not meant to be a thorough test of NavigationHistory,
        // but is only meant to test enough to make sure state is restored.
        // See NavigationHistoryTest for more thorough tests.
        for (int i = 0; i < NUM_NAVIGATIONS; ++i) {
            assertEquals(mUrls[i], history.getItemAt(i).getOriginalUrl());
            assertEquals(mUrls[i], history.getItemAt(i).getUrl());
            assertEquals(TITLES[i], history.getItemAt(i).getTitle());
        }
    }

    private void saveAndRestoreStateOnUiThread() throws Throwable {
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                Bundle bundle = new Bundle();
                mXWalkView.saveState(bundle);
                mRestoreXWalkView.restoreState(bundle);
            }
        });
    }

    @SmallTest
    @Feature({"SaveRestoreState"})
    public void testSaveRestoreStateWithTitle() throws Throwable {
        setServerResponseAndLoad(1);
        saveAndRestoreStateOnUiThread();
        assertTrue(pollOnUiThread(new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                // TODO(hengzhi): add the judge about updated title.
                return TITLES[0].equals(mXWalkView.getTitle());
            }
        }));
    }

    //@SmallTest
    //@Feature({"SaveRestoreState"})
    @DisabledTest
    public void testSaveRestoreStateWithHistoryItemList() throws Throwable {
        setServerResponseAndLoad(NUM_NAVIGATIONS);
        saveAndRestoreStateOnUiThread();
        checkHistoryItemList(mRestoreXWalkView);
    }

    @SmallTest
    @Feature({"SaveRestoreState"})
    public void testRestoreFromInvalidStateFails() throws Throwable {
        final Bundle invalidState = new Bundle();
        // TODO(yongsheng): how to share this with
        // XWalkContent.SAVE_RESTORE_STATE_KEY?
        invalidState.putByteArray("XWALKVIEW_STATE",
                                  "invalid state".getBytes());
        boolean result = runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return mXWalkView.restoreState(invalidState);
            }
        });
        assertFalse(result);
    }

    @SmallTest
    @Feature({"SaveRestoreState"})
    public void testSaveStateForNoNavigationFails() throws Throwable {
        final Bundle state = new Bundle();
        boolean result = runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return mXWalkView.restoreState(state);
            }
        });
        assertFalse(result);
    }
}
