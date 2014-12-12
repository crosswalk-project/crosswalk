// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.Context;

import junit.framework.Assert;

class XWalkLibraryDelegate {
    protected Context mContext;
    protected XWalkLibraryListener mLibraryListener;

    public static XWalkLibraryDelegate newInstance(Context context, XWalkLibraryListener listener) {
        if (XWalkCoreWrapper.getInstance() == null) {
            Assert.fail("Must call XWalkCoreWrapper.init() first");
        }
        if (XWalkCoreWrapper.getInstance().isSharedMode()) {
            return new XWalkLibraryDelegate(context, listener);
        }
        return new XWalkLibraryDelegateEmbedded(context, listener);
    }

    protected XWalkLibraryDelegate(Context context, XWalkLibraryListener listener) {
        mContext = context;
        mLibraryListener = listener;
    }

    public boolean initLibrary() {
        if (mLibraryListener != null) mLibraryListener.onXWalkLibraryPrepared();
        return true;
    }
}
