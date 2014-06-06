// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import java.io.Serializable;

import org.chromium.content.browser.NavigationHistory;

/**
 * This class represents a navigation history for a XWalkView instance.
 * It's not thread-safe and should be only called on UI thread.
 */
public final class XWalkNavigationHistory implements Cloneable, Serializable {
    private NavigationHistory mHistory;
    private XWalkView mXWalkView;

    XWalkNavigationHistory(XWalkView view, NavigationHistory history) {
        mXWalkView = view;
        mHistory = history;
    }

    XWalkNavigationHistory(XWalkNavigationHistory history) {
        mXWalkView = history.mXWalkView;
        mHistory = history.mHistory;
    }

    /**
     * Total size of navigation history for the XWalkView.
     * @return the size of total navigation items.
     */
    public int size() {
        return mHistory.getEntryCount();
    }

    /**
     * Test whether there is an item at a specific index.
     * @param index the given index.
     * @return true if there is an item at the specific index.
     */
    public boolean hasItemAt(int index) {
        return index >=0 && index <= size() - 1;
    }

    /**
     * Get a specific item given by index.
     * @param index the given index.
     * @return the navigation item for the given index.
     */
    public XWalkNavigationItem getItemAt(int index) {
        return new XWalkNavigationItem(mHistory.getEntryAtIndex(index));
    }

    /**
     * Get the current item which XWalkView displays.
     * @return the current navigation item.
     */
    public XWalkNavigationItem getCurrentItem() {
        return getItemAt(getCurrentIndex());
    }

    /**
     * Test whether XWalkView can go back.
     * @return true if it can go back.
     */
    public boolean canGoBack() {
        return mXWalkView.canGoBack();
    }

    /**
     * Test whether XWalkView can go forward.
     * @return true if it can go forward.
     */
    public boolean canGoForward() {
        return mXWalkView.canGoForward();
    }

    /**
     * The direction for web page navigation.
     * @deprecated use const int value instead.
     */
    public enum Direction {
        /** The backward direction for web page navigation. */
        BACKWARD,
        /** The forward direction for web page navigation. */
        FORWARD
    }

    /** The backward direction for web page navigation. */
    public final static int NAVIGATION_BACKWARD = 1;
    /** The forward direction for web page navigation. */
    public final static int NAVIGATION_FORWARD = 2;

    /**
     * Navigates to the specified step from the current navigation item.
     * Do nothing if the offset is out of bound.
     * @param direction the direction of navigation.
     * @param steps go back or foward with a given steps.
     * @deprecated use navigate(int direction, int steps) instead.
     */
    public void navigate(Direction direction, int steps) {
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
     * Navigates to the specified step from the current navigation item.
     * Do nothing if the offset is out of bound.
     * @param direction the direction of navigation.
     * @param steps go back or foward with a given steps.
     */
    public void navigate(int direction, int steps) {
        switch(direction) {
            case NAVIGATION_FORWARD:
                mXWalkView.navigateTo(steps);
                break;
            case NAVIGATION_BACKWARD:
                mXWalkView.navigateTo(-steps);
                break;
            default:
                break;
        }
    }

    /**
     * Get the index for current navigation item.
     * @return current index in the navigation history.
     */
    public int getCurrentIndex() {
        return mHistory.getCurrentEntryIndex();
    }

    /**
     * Clear all history owned by this XWalkView.
     */
    public void clear() {
        mXWalkView.clearHistory();
    }

    protected synchronized XWalkNavigationHistory clone() {
        return new XWalkNavigationHistory(this);
    }
}
