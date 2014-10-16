// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.content.res.Resources;

/**
 * XWalkActiviy is to support cross package resource loading.
 * It provides method to allow overriding getResources() behavior.
 */
public abstract class XWalkActivity extends Activity {

    @Override
    public Resources getResources() {
        return getApplicationContext().getResources();
    }
}
