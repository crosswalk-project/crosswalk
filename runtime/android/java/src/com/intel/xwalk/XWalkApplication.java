// Copyright (c) 2012 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package com.intel.xwalk;

import android.app.Application;

import org.chromium.base.PathUtils;
import org.chromium.content.browser.ResourceExtractor;

public class XWalkApplication extends Application {

    private static final String[] MANDATORY_PAK_FILES = new String[] {"xwalk.pak"};
    private static final String PRIVATE_DATA_DIRECTORY_SUFFIX = "xwalk";

    @Override
    public void onCreate() {
        super.onCreate();

        ResourceExtractor.setMandatoryPaksToExtract(MANDATORY_PAK_FILES);
        PathUtils.setPrivateDataDirectorySuffix(PRIVATE_DATA_DIRECTORY_SUFFIX);
    }
}

