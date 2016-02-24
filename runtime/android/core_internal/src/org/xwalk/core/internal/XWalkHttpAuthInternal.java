// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

/**
 * This interface is used when XWalkResourceClientInternal offers a http auth
 * request (cancel, proceed) to enable the client to handle the request in their
 * own way. XWalkResourceClientInternal will offer an object that implements
 * this interface to the client and when the client has handled the request,
 * it must either callback with proceed() or cancel() to allow processing to
 * continue.
 */
@XWalkAPI(instance = XWalkHttpAuthHandlerInternal.class)
public interface XWalkHttpAuthInternal {
    @XWalkAPI
    public void proceed(String username, String password);

    @XWalkAPI
    public void cancel();

    @XWalkAPI
    public boolean isFirstAttempt();
}
