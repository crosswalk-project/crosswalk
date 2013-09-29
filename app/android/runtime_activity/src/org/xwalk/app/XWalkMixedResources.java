// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app;

import android.content.res.Resources;
import android.graphics.drawable.Drawable;

/**
 * XWalkMixedResources is used to combine the resources
 * from two different packages.
 * 
 * TODO(wang16): Add more override functions (e.g. getColor/Boolean).
 */
public class XWalkMixedResources extends Resources {

    private Resources mExtend;

    XWalkMixedResources(Resources base, Resources extend) {
        super(base.getAssets(), base.getDisplayMetrics(),
                base.getConfiguration());
        mExtend = extend;
    }

    @Override
    public CharSequence getText(int id) throws NotFoundException {
        try {
            return mExtend.getText(id);
        } catch (NotFoundException e) {
            return super.getText(id);
        }
    }

    @Override
    public Drawable getDrawable(int id) throws NotFoundException {
        try {
            return mExtend.getDrawable(id);
        } catch (NotFoundException e) {
            return super.getDrawable(id);
        }
    }
}
