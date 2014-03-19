// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import org.chromium.content.browser.NavigationEntry;

/**
 * Represent a navigation item and managed in XWalkNavigationHistory.
 */
public final class XWalkNavigationItem implements Cloneable {
    private NavigationEntry mEntry;

    XWalkNavigationItem(NavigationEntry entry) {
        mEntry = entry;
    }

    XWalkNavigationItem(XWalkNavigationItem item) {
        mEntry = item.mEntry;
    }

    /**
     * Get the url of current navigation item.
     * @return the string of the url.
     */
    public String getUrl() {
        return mEntry.getUrl();
    }

    /**
     * Get the original url of current navigation item.
     * @return the string of the original url.
     */
    public String getOriginalUrl() {
        return mEntry.getOriginalUrl();
    }

    /**
     * Get the title of current navigation item.
     * @return the string of the title.
     */
    public String getTitle() {
        return mEntry.getTitle();
    }

    protected synchronized XWalkNavigationItem clone() {
        return new XWalkNavigationItem(this);
    }
}
