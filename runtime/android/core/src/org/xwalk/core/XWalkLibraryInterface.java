// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

/**
 * Interface used by the Crosswalk runtime
 */
interface XWalkLibraryInterface {
    /**
     * The Crosswalk runtime is pending.
     */
    public static final int STATUS_PENDING = 0;

    /**
     * The Crosswalk runtime matches the application.
     */
    public static final int STATUS_MATCH = 1;

    /**
     * The Crosswalk runtime is not found.
     */
    public static final int STATUS_NOT_FOUND = 2;

    /**
     * The version of the Crosswalk runtime is older than the application.
     */
    public static final int STATUS_OLDER_VERSION = 3;

    /**
     * The version of the Crosswalk runtime is newer than the application.
     */
    public static final int STATUS_NEWER_VERSION = 4;

    /**
     * Missing certain necessary native library
     */
    public static final int STATUS_INCOMPLETE_LIBRARY = 5;

    /**
     * Mismatch of CPU Architecture for the Crosswalk Runtime.
     */
    public static final int STATUS_ARCHITECTURE_MISMATCH = 6;

    /**
     * The Crosswalk signature verification failed.
     */
    public static final int STATUS_SIGNATURE_CHECK_ERROR = 7;

    /**
     * The Crosswalk Runtime doesn't match the application.
     */
    public static final int STATUS_RUNTIME_MISMATCH = 8;

    public static final String XWALK_CORE_PACKAGE = "org.xwalk.core";
    public static final String XWALK_CORE64_PACKAGE = "org.xwalk.core64";
    public static final String XWALK_CORE_IA_PACKAGE = "org.xwalk.core.ia";
    public static final String XWALK_CORE64_IA_PACKAGE = "org.xwalk.core64.ia";
}
