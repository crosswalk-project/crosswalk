// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_NET_INIT_NATIVE_CALLBACK_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_NET_INIT_NATIVE_CALLBACK_H_

#include <memory>

#include "base/memory/ref_counted.h"

namespace base {
class SingleThreadTaskRunner;
}

namespace net {
class CookieStore;
}  // namespace net

namespace xwalk {

// Gets the TaskRunner that the CookieStore must be called on.
scoped_refptr<base::SingleThreadTaskRunner> GetCookieStoreTaskRunner();

// Gets a pointer to the CookieStore managed by the CookieManager.
// The CookieStore is never deleted. May only be called on the
// CookieStore's TaskRunner.
net::CookieStore* GetCookieStore();

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_NET_INIT_NATIVE_CALLBACK_H_
