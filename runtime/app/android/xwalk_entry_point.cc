// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/android/jni_android.h"
#include "base/android/jni_registrar.h"
#include "content/public/app/android_library_loader_hooks.h"
#include "content/public/app/content_main.h"
#include "content/public/app/content_main_delegate.h"
#include "xwalk/runtime/app/android/xwalk_main_delegate_android.h"

// This is called by the VM when the shared library is first loaded.
JNI_EXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  base::android::InitVM(vm);
  JNIEnv* env = base::android::AttachCurrentThread();
  if (!content::RegisterLibraryLoaderEntryHook(env))
    return -1;

  content::SetContentMainDelegate(new xwalk::XWalkMainDelegateAndroid());

  return JNI_VERSION_1_4;
}
