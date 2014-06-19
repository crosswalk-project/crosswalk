// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import android.content.Intent;
import android.os.Bundle;

public class XWalkUIClientAPI extends XWalkEmbeddingAPIList {

    public XWalkUIClientAPI() {
        super(Intent.CATEGORY_TEST);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }
}
