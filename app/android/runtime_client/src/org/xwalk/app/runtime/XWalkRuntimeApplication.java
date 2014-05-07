// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app.runtime;

import android.app.Application;
import android.content.res.Resources;

/**
 * XWalkRuntimeApplication is to support cross package resource loading.
 * It provides method to allow overriding getResources() behavior.
 */
public class XWalkRuntimeApplication extends Application {
    private Resources mRes = null;

    @Override
    public Resources getResources() {
        return mRes == null ? super.getResources() : mRes;
    }

    void addResource(Resources res) {
        if (mRes != null) return;
        mRes = new XWalkMixedResources(super.getResources(), res);
    }
}
