// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import org.xwalk.core.internal.XWalkNavigationItemInternal;

/**
 * This class represents a navigation item and is managed in XWalkNavigationHistory.
 */
public final class XWalkNavigationItem extends XWalkNavigationItemInternal {

    public XWalkNavigationItem(XWalkNavigationItemInternal internal) {
        super(internal);
    }

    /**
     * Get the url of current navigation item.
     * @return the string of the url.
     */
    public String getUrl() {
        return super.getUrl();
    }

    /**
     * Get the original url of current navigation item.
     * @return the string of the original url.
     */
    public String getOriginalUrl() {
        return super.getOriginalUrl();
    }

    /**
     * Get the title of current navigation item.
     * @return the string of the title.
     */
    public String getTitle() {
        return super.getTitle();
    }
}
