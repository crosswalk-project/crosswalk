// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app.runtime;

/**
 * This class is to consolidate the exceptions happen when
 * the runtime client is trying to invoke runtime library
 * through reflection.
 * 
 * The exception will be set different label to identify which
 * stage the exception happened in.
 *
 * The exception handler for the runtime client will take
 * different action based on the type of the exception.
 */
public class XWalkRuntimeLibraryException extends Exception {
    public final static int XWALK_RUNTIME_LIBRARY_NOT_INSTALLED = 1;
    public final static int XWALK_RUNTIME_LIBRARY_NOT_UP_TO_DATE_CRITICAL = 2;
    public final static int XWALK_RUNTIME_LIBRARY_NOT_UP_TO_DATE_WARNING = 3;
    public final static int XWALK_RUNTIME_LIBRARY_LOAD_FAILED = 4;
    public final static int XWALK_RUNTIME_LIBRARY_INVOKE_FAILED = 5;
    
    private int mType;
    private Exception mOriginException;
    
    XWalkRuntimeLibraryException(int type, Exception originException) {
        mType = XWALK_RUNTIME_LIBRARY_NOT_INSTALLED;
        mOriginException = originException;
    }
    
    XWalkRuntimeLibraryException(int type) {
        mType = type;
        mOriginException = null;
    }
    
    XWalkRuntimeLibraryException() {
        mType = XWALK_RUNTIME_LIBRARY_NOT_INSTALLED;
        mOriginException = null;
    }
    
    public int getType() {
        return mType;
    }
    
    public Exception getOriginException() {
        return mOriginException;
    }
}
