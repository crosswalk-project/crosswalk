// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_ANDROID_XWALK_JNI_REGISTRAR_H_
#define XWALK_RUNTIME_ANDROID_XWALK_JNI_REGISTRAR_H_

#include <jni.h>

namespace xwalk {
namespace android {

// Register all JNI bindings necessary for XWalk.
bool RegisterXWalkJni(JNIEnv* env);

}  // namespace android
}  // namespace xwalk

#endif  // XWALK_RUNTIME_ANDROID_XWALK_JNI_REGISTRAR_H_

