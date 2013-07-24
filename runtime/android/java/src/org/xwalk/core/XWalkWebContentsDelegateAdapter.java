// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.util.Log;

public class XWalkWebContentsDelegateAdapter extends XWalkWebContentsDelegate {

    private XWalkContentsClient mXWalkContentsClient;

    public XWalkWebContentsDelegateAdapter(XWalkContentsClient client) {
        mXWalkContentsClient = client;
    }

    @Override
    public void onLoadProgressChanged(int progress) {
        if (mXWalkContentsClient != null)
            mXWalkContentsClient.onProgressChanged(progress);
    }

    @Override
    public boolean addNewContents(boolean isDialog, boolean isUserGesture) {
        // TODO: implement.
        return false;
    }

    @Override
    public void closeContents() {
        // TODO: implement.
    }

    @Override
    public void activateContents() {
        // TODO: implement.
    }
}
