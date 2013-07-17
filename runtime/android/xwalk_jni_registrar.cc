// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/android/xwalk_jni_registrar.h"

#include "base/android/jni_android.h"
#include "base/android/jni_registrar.h"

namespace {

static base::android::RegistrationMethod kXWalkRegistrationMethods[] = {
};

}  // namespace

namespace xwalk {
namespace android {

bool RegisterXWalkJni(JNIEnv* env) {
  return RegisterNativeMethods(env, kXWalkRegistrationMethods, 0 /* arraysize(kXWalkRegistrationMethods) */);
}

}  // namespace android
}  // namespace xwalk

