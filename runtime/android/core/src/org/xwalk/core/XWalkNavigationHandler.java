// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import org.chromium.components.navigation_interception.NavigationParams;

public interface XWalkNavigationHandler {

    /**
     * Handles the navigation request.
     * @param params The navigation parameters.
     * @return true if the navigation request is handled.
     */
    boolean handleNavigation(NavigationParams params);
}
