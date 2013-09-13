// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app.runtime.extension;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.util.AttributeSet;
import android.widget.FrameLayout;

import java.lang.reflect.Method;
import java.util.StringTokenizer;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.xwalk.app.runtime.CrossPackageWrapper;

/**
 * This is the extension context used by external extensions. It'll be created
 * by runtime side.
 */
public final class XWalkExtensionContextClient extends CrossPackageWrapper {
    private final static String EXTENSION_CLASS_NAME =
            "org.xwalk.runtime.extension.XWalkExtensionContextWrapper";
    private Object mInstance;
    private Method mGetContext;
    private Method mGetActivity;

    /**
     * It's called by runtime side.
     */
    public XWalkExtensionContextClient(Activity activity, Object instance) {
        super(activity, EXTENSION_CLASS_NAME, null, String.class, String.class,
                instance.getClass());

        mInstance = instance;
        mGetActivity = lookupMethod("getActivity");
        mGetContext = lookupMethod("getContext");
    }

    /**
     * Get the current Android Activity. Used by XWalkExtensionClient.
     * @return the current Android Activity.
     */
    public Activity getActivity() {
        return (Activity) invokeMethod(mGetActivity, mInstance);
    }

    /**
     * Get the current Android Context. Used by XWalkExtensionClient.
     * @return the current Android Context.
     */
    public Context getContext() {
        return (Context) invokeMethod(mGetContext, mInstance);
    }

    /**
     * Get the object of the runtime side.
     * @return the object of the runtime side.
     */
    public Object getInstance() {
        return mInstance;
    }
}
