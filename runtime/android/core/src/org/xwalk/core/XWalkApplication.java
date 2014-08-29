// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Application;
import android.content.Context;
import android.content.res.Resources;

/**
 * XWalkApplication is to support cross package resource loading.
 * It provides method to allow overriding getResources() behavior.
 */
public class XWalkApplication extends Application {
    private static XWalkApplication gApp = null;
    private Resources mRes = null;

    @Override
    public void onCreate(){
        super.onCreate();
        gApp = this;
    }

    @Override
    public Resources getResources() {
        return mRes == null ? super.getResources() : mRes;
    }

    void addResource(Resources res) {
        if (mRes != null) return;
        mRes = new XWalkMixedResources(super.getResources(), res);
    }

    static XWalkApplication getApplication() {
        return gApp;
    }
}
