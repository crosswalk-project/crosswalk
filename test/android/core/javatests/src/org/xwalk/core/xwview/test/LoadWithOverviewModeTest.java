// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.test;

import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;

/**
 * Test suite for LoadWithOverviewMode.
 */
public class LoadWithOverviewModeTest extends XWalkViewTestBase {

    @Override
    public void setUp() throws Exception {
        super.setUp();
    }

    @SmallTest
    @Feature({"LoadWithOverviewMode"})
    public void testLoadWithOverviewModeWithTwoViews() throws Throwable {
        ViewPair views = createViews();
        runPerViewSettingsTest(
                new XWalkSettingsLoadWithOverviewModeTestHelper(
                        views.getView0(), views.getBridge0(), false),
                new XWalkSettingsLoadWithOverviewModeTestHelper(
                        views.getView1(), views.getBridge1(), false));
    }

    @SmallTest
    @Feature({"LoadWithOverviewMode"})
    public void testLoadWithOverviewModeViewportTagWithTwoViews() throws Throwable {
        ViewPair views = createViews();
        runPerViewSettingsTest(
                new XWalkSettingsLoadWithOverviewModeTestHelper(
                        views.getView0(), views.getBridge0(), true),
                new XWalkSettingsLoadWithOverviewModeTestHelper(
                        views.getView1(), views.getBridge1(), true));
    }
}
