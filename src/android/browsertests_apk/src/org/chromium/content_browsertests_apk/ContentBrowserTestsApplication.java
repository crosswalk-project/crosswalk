// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.content_browsertests_apk;

import android.app.Application;

import org.chromium.base.PathUtils;
import org.chromium.content.app.LibraryLoader;
import org.chromium.content.browser.ResourceExtractor;

public class ContentBrowserTestsApplication extends Application {

    private static final String[] MANDATORY_PAK_FILES = new String[] {"content_shell.pak"};
    private static final String PRIVATE_DATA_DIRECTORY_SUFFIX = "content_shell";

    @Override
    public void onCreate() {
        super.onCreate();
        initializeApplicationParameters();
    }

    public static void initializeApplicationParameters() {
        ResourceExtractor.setMandatoryPaksToExtract(MANDATORY_PAK_FILES);
        PathUtils.setPrivateDataDirectorySuffix(PRIVATE_DATA_DIRECTORY_SUFFIX);
    }

}
