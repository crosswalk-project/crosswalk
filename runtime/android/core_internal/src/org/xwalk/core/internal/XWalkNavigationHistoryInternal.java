// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import java.io.Serializable;

import org.chromium.content.browser.NavigationHistory;

/**
 * This class represents a navigation history for a XWalkViewInternal instance.
 * It's not thread-safe and should be only called on UI thread.
 */
public class XWalkNavigationHistoryInternal implements Cloneable, Serializable {
    private NavigationHistory mHistory;
    private XWalkViewInternal mXWalkView;

    XWalkNavigationHistoryInternal(XWalkViewInternal view, NavigationHistory history) {
        mXWalkView = view;
        mHistory = history;
    }

    public XWalkNavigationHistoryInternal(XWalkNavigationHistoryInternal history) {
        mXWalkView = history.mXWalkView;
        mHistory = history.mHistory;
    }

    /**
     * Total size of navigation history for the XWalkViewInternal.
     * @return the size of total navigation items.
     * @since 1.0
     */
    public int size() {
        return mHistory.getEntryCount();
    }

    /**
     * Test whether there is an item at a specific index.
     * @param index the given index.
     * @return true if there is an item at the specific index.
     * @since 1.0
     */
    public boolean hasItemAt(int index) {
        return index >=0 && index <= size() - 1;
    }

    /**
     * Get a specific item given by index.
     * @param index the given index.
     * @return the navigation item for the given index.
     * @since 1.0
     */
    public XWalkNavigationItemInternal getItemAt(int index) {
        if (index < 0 || index >= size()) return null;
        return new XWalkNavigationItemInternal(mHistory.getEntryAtIndex(index));
    }

    /**
     * Get the current item which XWalkViewInternal displays.
     * @return the current navigation item.
     * @since 1.0
     */
    public XWalkNavigationItemInternal getCurrentItem() {
        return getItemAt(getCurrentIndex());
    }

    /**
     * Test whether XWalkViewInternal can go back.
     * @return true if it can go back.
     * @since 1.0
     */
    public boolean canGoBack() {
        return mXWalkView.canGoBack();
    }

    /**
     * Test whether XWalkViewInternal can go forward.
     * @return true if it can go forward.
     * @since 1.0
     */
    public boolean canGoForward() {
        return mXWalkView.canGoForward();
    }

    /**
     * The direction for web page navigation.
     * @since 1.0
     */
    public enum DirectionInternal {
        /** The backward direction for web page navigation. */
        BACKWARD,
        /** The forward direction for web page navigation. */
        FORWARD
    }

    /**
     * Navigates to the specified step from the current navigation item.
     * Do nothing if the offset is out of bound.
     * @param direction the direction of navigation.
     * @param steps go back or foward with a given steps.
     * @since 1.0
     */
    public void navigate(DirectionInternal direction, int steps) {
        switch(direction) {
            case FORWARD:
                mXWalkView.navigateTo(steps);
                break;
            case BACKWARD:
                mXWalkView.navigateTo(-steps);
                break;
            default:
                break;
        }
    }

    /**
     * Get the index for current navigation item.
     * @return current index in the navigation history.
     * @since 1.0
     */
    public int getCurrentIndex() {
        return mHistory.getCurrentEntryIndex();
    }

    /**
     * Clear all history owned by this XWalkViewInternal.
     * @since 1.0
     */
    public void clear() {
        mXWalkView.clearHistory();
    }

    protected synchronized XWalkNavigationHistoryInternal clone() {
        return new XWalkNavigationHistoryInternal(this);
    }
}
