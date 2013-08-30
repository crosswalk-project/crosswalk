// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

public interface DownloadListener {

    public abstract void onDownloadStart(String url, String userAgent,
            String contentDisposition, String mimetype, long contentLength);
}
