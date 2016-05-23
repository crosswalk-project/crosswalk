// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

/**
  * Interface to listen for find results.
*/
@XWalkAPI(createExternally = true)
public abstract class XWalkFindListenerInternal {
    /**
     * Notifies the listener about progress made by a find operation.
     *
     * @param activeMatchOrdinal the zero-based ordinal of the currently selected match
     * @param numberOfMatches how many matches have been found
     * @param isDoneCounting whether the find operation has actually completed. The listener may be
     *                       notified multiple times while the operation is underway, and the
     *                       numberOfMatches value should not be considered final unless
     *                       isDoneCounting is true.
     * @since 7.0
     */
    @XWalkAPI
    public abstract void onFindResultReceived(int activeMatchOrdinal, int numberOfMatches,
            boolean isDoneCounting);
}
