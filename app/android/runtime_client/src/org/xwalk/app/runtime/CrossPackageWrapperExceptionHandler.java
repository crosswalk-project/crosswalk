// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app.runtime;

public interface CrossPackageWrapperExceptionHandler {
    public void onException(Exception e);
    public void onException(String msg);
}
