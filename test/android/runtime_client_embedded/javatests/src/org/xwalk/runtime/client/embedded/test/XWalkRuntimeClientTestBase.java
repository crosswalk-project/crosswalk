// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.client.embedded.test;

import android.app.Activity;
import android.test.ActivityInstrumentationTestCase2;

import org.xwalk.app.runtime.XWalkRuntimeClient;
import org.xwalk.test.util.XWalkRuntimeClientTestGeneric;
import org.xwalk.test.util.XWalkRuntimeClientTestUtilBase;
import org.xwalk.test.util.XWalkRuntimeClientTestUtilBase.PageStatusCallback;

public class XWalkRuntimeClientTestBase
        extends XWalkRuntimeClientTestGeneric<XWalkRuntimeClientTestRunnerActivity> {

    public XWalkRuntimeClientTestBase() {
        super(XWalkRuntimeClientTestRunnerActivity.class);
    }

    @Override
    public void postSetUp() {
        getActivity().addView(getTestUtil().getTestedView().getViewForTest());
        getActivity().registerBroadcastReceiver(getTestUtil().getTestedView());
    }
}
