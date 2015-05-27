// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

/**
 * Interface used by the Crosswalk runtime
 */
interface XWalkLibraryInterface {
    /**
     * The Crosswalk runtime matches current application
     */
    public static final int STATUS_MATCH = 0;

    /**
     * The Crosswalk runtime is not found
     */
    public static final int STATUS_NOT_FOUND = 1;

    /**
     * Mismatch of CPU Architecture for the Crosswalk Runtime
     */
    public static final int STATUS_ARCHITECTURE_MISMATCH = 2;

    /**
     * The Crosswalk signature verification failed
     */
    public static final int STATUS_SIGNATURE_CHECK_ERROR = 3;

    /**
     * The version of the Crosswalk runtime is older than current application
     */
    public static final int STATUS_OLDER_VERSION = 4;

    /**
     * The version of the Crosswalk runtime is newer than current application
     */
    public static final int STATUS_NEWER_VERSION = 5;
}
