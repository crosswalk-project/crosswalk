// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.graphics.Bitmap;

import org.chromium.content.browser.NavigationEntry;

// Wrap NavigationEntry in content as WebHistoryItem.
public class WebHistoryItem implements Cloneable {
    private static int mNextId = 0;
    private int mId;
    private NavigationEntry mEntry;
    public WebHistoryItem(NavigationEntry entry) {
        mId = mNextId++;
        mEntry = entry;
    }

    public WebHistoryItem(WebHistoryItem item) {
        mId = item.mId;
        mEntry = item.mEntry;
    }

    @Deprecated
    public int getId() {
        return mId;
    }

    public String getUrl() {
        return mEntry.getUrl();
    }

    public String getOriginalUrl() {
        return mEntry.getOriginalUrl();
    }

    public String getTitle() {
        return mEntry.getTitle();
    }

    public Bitmap getFavicon() {
        return mEntry.getFavicon();
    }

    protected synchronized WebHistoryItem clone() {
        return new WebHistoryItem(this);
    }
}
