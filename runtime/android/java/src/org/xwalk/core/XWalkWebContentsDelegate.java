// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import org.chromium.base.CalledByNative;
import org.chromium.base.JNINamespace;
import org.chromium.components.web_contents_delegate_android.WebContentsDelegateAndroid;

@JNINamespace("xwalk")
public abstract class XWalkWebContentsDelegate extends WebContentsDelegateAndroid {
    @CalledByNative
    public abstract boolean addNewContents(boolean isDialog, boolean isUserGesture);

    @CalledByNative
    public abstract void closeContents();

    @CalledByNative
    public abstract void activateContents();

    @CalledByNative
    public abstract void rendererUnresponsive();

    @CalledByNative
    public abstract void rendererResponsive();

    @CalledByNative
    public void updatePreferredSize(int widthCss, int heightCss) {
    }
}
