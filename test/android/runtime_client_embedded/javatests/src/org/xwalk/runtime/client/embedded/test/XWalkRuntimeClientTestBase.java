// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.client.embedded.test;

import android.app.Activity;
import android.test.ActivityInstrumentationTestCase2;

import org.xwalk.app.runtime.XWalkRuntimeClient;
import org.xwalk.test.util.XWalkRuntimeClientUtilInterface;
import org.xwalk.test.util.XWalkRuntimeClientUtilInterface.PageStatusCallback;

public class XWalkRuntimeClientTestBase
        extends ActivityInstrumentationTestCase2<XWalkRuntimeClientTestRunnerActivity> {
    private XWalkRuntimeClient mRuntimeView;
    XWalkRuntimeClientUtilInterface mUtilInterface;

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        final Activity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            @Override
            public void run() {
                if (mRuntimeView == null || mRuntimeView.get() == null) {
                    mRuntimeView = new XWalkRuntimeClient(activity, null, null);
                }
                mRuntimeView.onCreate();
                mUtilInterface = new XWalkRuntimeClientUtilInterface(mRuntimeView, getInstrumentation());
                PageStatusCallback callback = mUtilInterface.new PageStatusCallback();
                mRuntimeView.setCallbackForTest((Object)callback);
                getActivity().addView(mRuntimeView.getViewForTest());
            }
        });
    }

    public XWalkRuntimeClientTestBase() {
        super(XWalkRuntimeClientTestRunnerActivity.class);
    }

    public XWalkRuntimeClientUtilInterface getUtilInterface() {
        return mUtilInterface;
    }
}
