// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import org.chromium.content.browser.NavigationEntry;

/**
 * This class represents a navigation item and is managed in XWalkNavigationHistoryInternal.
 */
public class XWalkNavigationItemInternal implements Cloneable {
    private NavigationEntry mEntry;

    XWalkNavigationItemInternal(NavigationEntry entry) {
        mEntry = entry;
    }

    public XWalkNavigationItemInternal(XWalkNavigationItemInternal item) {
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

    protected synchronized XWalkNavigationItemInternal clone() {
        return new XWalkNavigationItemInternal(this);
    }
}
