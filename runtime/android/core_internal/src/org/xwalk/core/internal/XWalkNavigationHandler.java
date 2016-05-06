// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import org.chromium.components.navigation_interception.NavigationParams;

interface XWalkNavigationHandler {

    /**
     * Handles the navigation request.
     * @param params The navigation parameters.
     * @return true if the navigation request is handled.
     */
    boolean handleNavigation(NavigationParams params);

    /**
     * Gets the fallback url for special chemes, e.g. intent://.
     * @return the fallback url if it was specified.
     */
    String getFallbackUrl();

    /**
     * Resets the fallback url to null.
     */
    void resetFallbackUrl();
}
