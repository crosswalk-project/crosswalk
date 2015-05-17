// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

/**
 * Contains all of the command line switches for crosswalk
 */
public abstract class XWalkSwitches {
    // Native switch - xwalk_switches::kXWalkProfileName
    public static final String PROFILE_NAME = "profile-name";

    public static final String DISABLE_GPU_RASTERIZATION = "disable-gpu-rasterization";

    // Prevent instantiation.
    private XWalkSwitches() {}
}
