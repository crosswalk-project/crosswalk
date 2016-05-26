// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;

import org.chromium.base.test.util.Feature;

/**
 * Test suite for UseWideViewport.
 */
public class UseWideViewportTest extends XWalkViewTestBase {

    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @SmallTest
    @Feature({"WideViewport"})
    public void testUseWideViewportWithTwoViews() throws Throwable {
        ViewPair views = createViews();
        setQuirksModeByXWalkView(true, views.getView0());
        setQuirksModeByXWalkView(true, views.getView1());
        runPerViewSettingsTest(
                new XWalkSettingsUseWideViewportTestHelper(views.getView0(), views.getBridge0()),
                new XWalkSettingsUseWideViewportTestHelper(views.getView1(), views.getBridge1()));
    }

    @SmallTest
    @Feature({"WideViewport"})
    public void testUseWideViewportWithTwoViewsNoQuirks() throws Throwable {
        ViewPair views = createViews();
        runPerViewSettingsTest(
                new XWalkSettingsUseWideViewportTestHelper(views.getView0(), views.getBridge0()),
                new XWalkSettingsUseWideViewportTestHelper(views.getView1(), views.getBridge1()));
    }
}
