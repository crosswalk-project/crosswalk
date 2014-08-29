// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.content.Context;
import android.util.AttributeSet;

/**
 * The XWalkView that allows to use Crosswalk's shared library.
 */
public class SharedXWalkView extends XWalkView {
    public SharedXWalkView(Context context, AttributeSet attrs,
            SharedXWalkExceptionHandler handler) {
        super(verifyActivity(context, handler), attrs);
    }

    public SharedXWalkView(Context context, Activity activity,
            SharedXWalkExceptionHandler handler) {
        super(context, verifyActivity(activity, handler));
    }

    private static Activity verifyActivity(Context context, SharedXWalkExceptionHandler handler) {
        assert context instanceof Activity;
        assert context.getApplicationContext() instanceof XWalkApplication;
        ReflectionHelper.allowCrossPackage();
        ReflectionHelper.setExceptionHandler(handler);
        return (Activity) context;
    }
}
