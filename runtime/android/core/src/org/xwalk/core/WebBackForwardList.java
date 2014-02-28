// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import java.io.Serializable;

import org.chromium.content.browser.NavigationHistory;

// Wrap NavigationHistory in content as WebBackForwardList.
public class WebBackForwardList implements Cloneable, Serializable {
    private NavigationHistory mHistory;

    public WebBackForwardList(NavigationHistory history) {
        mHistory = history;
    }

    public WebBackForwardList(WebBackForwardList list) {
        mHistory = list.mHistory;
    }

    public synchronized WebHistoryItem getCurrentItem() {
       return getItemAtIndex(getCurrentIndex());
    }

    public synchronized int getCurrentIndex() {
        return mHistory.getCurrentEntryIndex();
    }

    public synchronized WebHistoryItem getItemAtIndex(int index) {
        return new WebHistoryItem(mHistory.getEntryAtIndex(index));
    }

    public synchronized int getSize() {
        return mHistory.getEntryCount();
    }

    protected synchronized WebBackForwardList clone() {
        return new WebBackForwardList(this);
    }
}
