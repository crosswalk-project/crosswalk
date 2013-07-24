// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_APP_ANDROID_XWALK_JNI_REGISTRAR_H_
#define XWALK_RUNTIME_APP_ANDROID_XWALK_JNI_REGISTRAR_H_

#include <jni.h>

namespace xwalk {

// Register all JNI bindings necessary for xwalk.
bool RegisterJni(JNIEnv* env);

}  // namespace xwalk

#endif  // XWALK_RUNTIME_APP_ANDROID_XWALK_JNI_REGISTRAR_H_
