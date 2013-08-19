// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.test;

import android.test.ActivityInstrumentationTestCase2;

public class XWalkRuntimeTestBase
       extends ActivityInstrumentationTestCase2<XWalkRuntimeTestRunnerActivity> {

    @Override
    protected void setUp() throws Exception {
        super.setUp();
    }

    public XWalkRuntimeTestBase() {
        super(XWalkRuntimeTestRunnerActivity.class);
    }
}
