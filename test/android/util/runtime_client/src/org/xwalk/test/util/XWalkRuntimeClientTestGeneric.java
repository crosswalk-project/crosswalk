// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.test.util;

import android.app.Activity;
import android.content.Context;
import android.test.ActivityInstrumentationTestCase2;

import org.xwalk.app.runtime.XWalkRuntimeClient;
import org.xwalk.test.util.XWalkRuntimeClientTestUtilBase.PageStatusCallback;

public class XWalkRuntimeClientTestGeneric<T extends Activity>
        extends ActivityInstrumentationTestCase2<T> {
    private XWalkRuntimeClient mRuntimeView;
    XWalkRuntimeClientTestUtilBase mTestUtil;

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
                mRuntimeView.onResume();
                mTestUtil = new XWalkRuntimeClientTestUtilBase(mRuntimeView,
                        getInstrumentation());
                PageStatusCallback callback = mTestUtil.new PageStatusCallback();
                mRuntimeView.setCallbackForTest((Object)callback);
                postSetUp();
            }
        });
    }

    public void postSetUp() {
    }

    public XWalkRuntimeClientTestGeneric(Class<T> activityClass) {
        super(activityClass);
    }

    public XWalkRuntimeClientTestUtilBase getTestUtil() {
        return mTestUtil;
    }
}
