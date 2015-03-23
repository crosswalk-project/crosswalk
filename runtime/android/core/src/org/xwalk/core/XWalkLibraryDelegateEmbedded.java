// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.Context;

import junit.framework.Assert;

class XWalkLibraryDelegateEmbedded extends XWalkLibraryDelegate {
    public XWalkLibraryDelegateEmbedded(Context context, XWalkLibraryListener listener) {
        super(context, listener);
    }

    @Override
    public boolean initLibrary() {
        try {
            Class<?> clazz = XWalkLibraryDelegateEmbedded.class.getClassLoader().loadClass(
                    XWalkCoreWrapper.BRIDGE_PACKAGE + ".XWalkViewDelegate");
            ReflectMethod method = new ReflectMethod(null, clazz,
                    "loadXWalkLibrary", Context.class);
            method.invoke((Context) null);
        } catch (ClassNotFoundException e) {
            Assert.fail("Incompatible embedded core");
        } catch (RuntimeException e) {
            e.printStackTrace();
            return false;
        }

        return super.initLibrary();
    }
}
