// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.util.Log;

import junit.framework.Assert;

class XWalkReflectionHelper {
    public static void checkEmbeddedMode() {
        if (XWalkCoreWrapper.getInstance() != null ||
                XWalkCoreWrapper.getProvisionalInstance() != null) {
            return;
        }

        if (!XWalkCoreWrapper.checkEmbeddedMode()) {
            Assert.fail("Must extend XWalkActivity in shared mode");
        }
        XWalkCoreWrapper.init();

        XWalkLibraryDelegate delegate = XWalkLibraryDelegate.newInstance(null, null);
        if (!delegate.initLibrary()) {
            Assert.fail("XWalk library initialization failed");
        }
        Log.d("XWalkActivity", "Initialized embedded mode by reflection helper");
    }
}
