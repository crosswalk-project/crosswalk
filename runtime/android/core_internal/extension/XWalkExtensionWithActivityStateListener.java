// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.extension;

import java.lang.ref.WeakReference;

import org.chromium.base.ApplicationStatus;
import org.chromium.base.ApplicationStatus.ActivityStateListener;
import org.xwalk.core.internal.XWalkExtensionInternal;
import org.xwalk.core.internal.XWalkViewInternal;

import android.app.Activity;
import android.content.Context;

/**
 * Base class for internal extensions which cares about host activity's lifecycle.
 */
public abstract class XWalkExtensionWithActivityStateListener extends XWalkExtensionInternal {

    private class XWalkActivityStateListener implements ActivityStateListener {
        WeakReference<XWalkExtensionWithActivityStateListener> mExtensionRef;

        XWalkActivityStateListener(XWalkExtensionWithActivityStateListener extension) {
            mExtensionRef = new WeakReference<XWalkExtensionWithActivityStateListener>(extension);
        }

        @Override
        public void onActivityStateChange(Activity activity, int newState) {
            XWalkExtensionWithActivityStateListener extension = mExtensionRef.get();
            if (extension == null) return;
            extension.onActivityStateChange(activity, newState);
        }
    }

    private XWalkActivityStateListener mActivityStateListener;

    private void initActivityStateListener(Activity activity) {
        mActivityStateListener = new XWalkActivityStateListener(this);
        ApplicationStatus.registerStateListenerForActivity(mActivityStateListener, activity);
    }

    public abstract void onActivityStateChange(Activity activity, int newState);

    public XWalkExtensionWithActivityStateListener(String name, String jsApi, Activity activity) {
        super(name, jsApi);
        initActivityStateListener(activity);
    }

    public XWalkExtensionWithActivityStateListener(
            String name, String jsApi, String[] entryPoints, Activity activity) {
        super(name, jsApi, entryPoints);
        initActivityStateListener(activity);
    }
}
