// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.test.util;

import org.chromium.content.browser.test.util.CallbackHelper;

public class OnTitleUpdatedHelper extends CallbackHelper {
    private String mTitle;

    public void notifyCalled(String title) {
        mTitle = title;
        notifyCalled();
    }

    public String getTitle() {
        assert getCallCount() > 0;
        return mTitle;
    }
}
