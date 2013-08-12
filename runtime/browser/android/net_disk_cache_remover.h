// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_NET_DISK_CACHE_REMOVER_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_NET_DISK_CACHE_REMOVER_H_

namespace content {

class BrowserContext;

}  // namespace content

namespace xwalk {

// Clear all http disk cache for this renderer. This method is asynchronous and
// will noop if a previous call has not finished.
void RemoveHttpDiskCache(content::BrowserContext* browser_context,
                        int renderer_child_id);

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_NET_DISK_CACHE_REMOVER_H_
