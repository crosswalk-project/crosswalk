// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_COOKIE_MANAGER_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_COOKIE_MANAGER_H_

#include <jni.h>

namespace net {
class CookieMonster;
}  // namespace net

namespace xwalk {
class AwURLRequestJobFactory;

void SetCookieMonsterOnNetworkStackInit(net::CookieMonster* cookie_monster);

net::CookieMonster* GetCookieMonster();

bool RegisterCookieManager(JNIEnv* env);

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_COOKIE_MANAGER_H_
