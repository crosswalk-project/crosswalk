// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import org.xwalk.core.internal.XWalkNavigationHistoryInternal;
import org.xwalk.core.internal.XWalkNavigationHistoryInternal.DirectionInternal;
import org.xwalk.core.internal.XWalkNavigationItemInternal;

/**
 * This class represents a navigation history for a XWalkView instance.
 * It's not thread-safe and should be only called on UI thread.
 */
public final class XWalkNavigationHistory extends XWalkNavigationHistoryInternal {

    XWalkNavigationHistory(XWalkNavigationHistoryInternal internal) {
        super(internal);
    }

    /**
     * Total size of navigation history for the XWalkView.
     * @return the size of total navigation items.
     * @since 1.0
     */
    public int size() {
        return super.size();
    }

    /**
     * Test whether there is an item at a specific index.
     * @param index the given index.
     * @return true if there is an item at the specific index.
     * @since 1.0
     */
    public boolean hasItemAt(int index) {
        return super.hasItemAt(index);
    }

    /**
     * Get a specific item given by index.
     * @param index the given index.
     * @return the navigation item for the given index.
     * @since 1.0
     */
    public XWalkNavigationItem getItemAt(int index) {
        XWalkNavigationItemInternal item = super.getItemAt(index);
        if (item == null || item instanceof XWalkNavigationItem) {
            return (XWalkNavigationItem) item;
        }

        return new XWalkNavigationItem(item);
    }

    /**
     * Get the current item which XWalkView displays.
     * @return the current navigation item.
     * @since 1.0
     */
    public XWalkNavigationItem getCurrentItem() {
        XWalkNavigationItemInternal item = super.getCurrentItem();
        if (item == null || item instanceof XWalkNavigationItem) {
            return (XWalkNavigationItem) item;
        }

        return new XWalkNavigationItem(item);
    }

    /**
     * Test whether XWalkView can go back.
     * @return true if it can go back.
     * @since 1.0
     */
    public boolean canGoBack() {
        return super.canGoBack();
    }

    /**
     * Test whether XWalkView can go forward.
     * @return true if it can go forward.
     * @since 1.0
     */
    public boolean canGoForward() {
        return super.canGoForward();
    }

    /**
     * The direction for web page navigation.
     * @since 1.0
     */
    public enum Direction {
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
    public void navigate(Direction direction, int steps) {
        super.navigate(DirectionInternal.valueOf(direction.toString()), steps);
    }

    /**
     * Get the index for current navigation item.
     * @return current index in the navigation history.
     * @since 1.0
     */
    public int getCurrentIndex() {
        return super.getCurrentIndex();
    }

    /**
     * Clear all history owned by this XWalkView.
     * @since 1.0
     */
    public void clear() {
        super.clear();
    }
}
