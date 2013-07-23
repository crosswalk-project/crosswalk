// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/app/android/xwalk_jni_registrar.h"

#include "base/android/jni_android.h"
#include "base/android/jni_registrar.h"
#include "xwalk/runtime/browser/android/xwalk_content.h"
#include "xwalk/runtime/browser/android/xwalk_contents_client_bridge.h"

namespace xwalk {

static base::android::RegistrationMethod kXWalkRegisteredMethods[] = {
  // Register JNI for xwalk classes.
  { "XWalkContentsClientBridge", RegisterXWalkContentsClientBridge },
  { "XWalkContent", RegisterXWalkContent },
};

bool RegisterJni(JNIEnv* env) {
  return RegisterNativeMethods(env,
    kXWalkRegisteredMethods, arraysize(kXWalkRegisteredMethods));
}

}  // namespace xwalk
