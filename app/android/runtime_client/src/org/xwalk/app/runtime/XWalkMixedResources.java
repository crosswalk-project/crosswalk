// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app.runtime;

import android.content.res.Resources;
import android.content.res.TypedArray;
import android.content.res.XmlResourceParser;
import android.util.TypedValue;

/**
 * XWalkMixedResources is used to combine the resources
 * from two different packages.
 *
 * Chromium uses resources through:
 *   R.layout; R.id; R.string; R.dimen; R.drawable;
 *   R.attr; R.style; R.menu ; R.color
 * R.layout and R.menu is covered by Resources.getLayout()
 * R.string is covered by Resources.getText()
 * R.dimen, R.drawable and R.color is covered by getValue()
 *
 * For R.id, if it's used like findViewById(R.id.xxx), R.id.xxx
 * is const at compile time within library context. It works
 * if the view in hierachy has the same id, which needs inflate
 * with layout resource from library context. Layout is covered
 * by getLayout(), so R.id is OK.
 *
 * TODO(wang16):
 * For R.attr and R.style, I have no confidence that it's covered.
 * But the only place use this R.attr and R.style is "select" tag
 * which is verified working well with this MixedResources.
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
    public XmlResourceParser getLayout(int id) throws NotFoundException {
        try {
            return mExtend.getLayout(id);
        } catch (NotFoundException e) {
            return super.getLayout(id);
        }
    }

    @Override
    public void getValue(int id, TypedValue outValue, boolean resolveRefs) {
        try {
            mExtend.getValue(id, outValue, resolveRefs);
        } catch (NotFoundException e) {
            super.getValue(id, outValue, resolveRefs);
        }
    }

    @Override
    public void getValueForDensity(int id, int density, TypedValue outValue, boolean resolveRefs) {
        try {
            mExtend.getValueForDensity(id, density, outValue, resolveRefs);
        } catch (NotFoundException e) {
            super.getValueForDensity(id, density, outValue, resolveRefs);
        }
    }

    @Override
    public int getIdentifier(String name, String defType, String defPackage) {
        int id = mExtend.getIdentifier(name, defType, defPackage);
        return id != 0 ? id : super.getIdentifier(name, defType, defPackage);
    }
}
