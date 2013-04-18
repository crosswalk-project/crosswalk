// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAMEO_SRC_ANDROID_SHELL_JNI_REGISTRAR_H_
#define CAMEO_SRC_ANDROID_SHELL_JNI_REGISTRAR_H_

#include <jni.h>

namespace cameo {
namespace android {

// Register all JNI bindings necessary for content shell.
bool RegisterShellJni(JNIEnv* env);

}  // namespace android
}  // namespace cameo

#endif  // CAMEO_SRC_ANDROID_SHELL_JNI_REGISTRAR_H_