// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension.api.presentation;

import android.view.Display;
import android.view.View;

/**
 * A empty implementation for build version lower than API level 17.
 */
public class PresentationViewNull extends PresentationView {

    public PresentationViewNull() {
    }

    @Override
    public void show() {
    }

    @Override
    public void dismiss() {
    }

    @Override
    public void cancel() {
    }

    @Override
    public void setContentView(View contentView) {
    }

    @Override
    public Display getDisplay() {
        return null;
    }
}

