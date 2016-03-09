// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;
/**
 * Create an empty class in order to make Lite-17 compatible with Cordova
 * Plugin 1.7.0(which matches Crosswalk-18)
 */
@XWalkAPI(createExternally = true)
public class XWalkExternalExtensionManagerInternal {
    @XWalkAPI
    public XWalkExternalExtensionManagerInternal() {
    }

    /* empty function.*/
    @XWalkAPI
    public void loadExtension(String extensionPath) {
        return;
    }  
}
