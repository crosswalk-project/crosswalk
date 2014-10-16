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

    private static boolean initialized = false;

    public SharedXWalkView(XWalkActivity context, AttributeSet attrs,
            SharedXWalkExceptionHandler handler) {
        super(verifyActivity(context), attrs);
    }

    public SharedXWalkView(Context context, XWalkActivity activity) {
        super(context, verifyActivity(activity));
    }

    private static Activity verifyActivity(XWalkActivity context) {
        if (!initialized) initialize(context, null);
        return context;
    }

    public static void initialize(Context context, SharedXWalkExceptionHandler handler) {
        if (initialized) return;

        assert context.getApplicationContext() instanceof XWalkApplication;
        ReflectionHelper.allowCrossPackage();
        if (handler != null) ReflectionHelper.setExceptionHandler(handler);
        initialized = true;
    }

    public static boolean containsLibrary() {
        return !ReflectionHelper.shouldUseLibrary();
    }

    public static boolean isUsingLibrary() {
        return ReflectionHelper.isUsingLibrary();
    }
}
