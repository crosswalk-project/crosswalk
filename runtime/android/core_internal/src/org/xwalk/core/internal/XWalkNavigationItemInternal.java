// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import org.chromium.content_public.browser.NavigationEntry;

/**
 * This class represents a navigation item and is managed in XWalkNavigationHistoryInternal.
 */
@XWalkAPI(createInternally = true)
public class XWalkNavigationItemInternal implements Cloneable {
    private NavigationEntry mEntry;

    // Never use this constructor.
    // It is only used in XWalkNavigationItemBridge.
    XWalkNavigationItemInternal() {
        mEntry = null;
    }

    XWalkNavigationItemInternal(NavigationEntry entry) {
        mEntry = entry;
    }

    XWalkNavigationItemInternal(XWalkNavigationItemInternal item) {
        mEntry = item.mEntry;
    }

    /**
     * Get the url of current navigation item.
     * @return the string of the url.
     * @since 1.0
     */
    @XWalkAPI
    public String getUrl() {
        return mEntry.getUrl();
    }

    /**
     * Get the original url of current navigation item.
     * @return the string of the original url.
     * @since 1.0
     */
    @XWalkAPI
    public String getOriginalUrl() {
        return mEntry.getOriginalUrl();
    }

    /**
     * Get the title of current navigation item.
     * @return the string of the title.
     * @since 1.0
     */
    @XWalkAPI
    public String getTitle() {
        return mEntry.getTitle();
    }

    protected synchronized XWalkNavigationItemInternal clone() {
        return new XWalkNavigationItemInternal(this);
    }
}
