// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_ANDROID_SHELL_JNI_REGISTRAR_H_
#define CONTENT_SHELL_ANDROID_SHELL_JNI_REGISTRAR_H_

#include <jni.h>

namespace content {
namespace android {

// Register all JNI bindings necessary for content shell.
bool RegisterShellJni(JNIEnv* env);

}  // namespace android
}  // namespace content

#endif  // CONTENT_SHELL_ANDROID_SHELL_JNI_REGISTRAR_H_
