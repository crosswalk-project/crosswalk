// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/android/shell_jni_registrar.h"

#include "base/android/jni_android.h"
#include "base/android/jni_registrar.h"
#include "content/shell/android/shell_manager.h"
#include "content/shell/shell.h"

namespace {

static base::android::RegistrationMethod kShellRegistrationMethods[] = {
  { "Shell", content::Shell::Register },
  { "ShellManager", content::RegisterShellManager },
};

}  // namespace

namespace content {
namespace android {

bool RegisterShellJni(JNIEnv* env) {
  return RegisterNativeMethods(env, kShellRegistrationMethods,
                               arraysize(kShellRegistrationMethods));
}

}  // namespace android
}  // namespace content
