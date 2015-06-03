// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/android/jni_android.h"
#include "base/android/jni_registrar.h"
#include "base/android/library_loader/library_loader_hooks.h"
#include "content/public/app/android_library_loader_hooks.h"
#include "content/public/app/content_main.h"
#include "content/public/app/content_main_delegate.h"
#include "xwalk/runtime/app/android/xwalk_jni_registrar.h"
#include "xwalk/runtime/app/android/xwalk_main_delegate_android.h"
#if defined(USE_COCOS2D)
#include "third_party/WebKit/Source/core/cocos2d/cocos2dx/platform/android/blinkjni/Cocos2dxJniHelper.h"
#endif

// This is called by the VM when the shared library is first loaded.
JNI_EXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  base::android::SetLibraryLoadedHook(&content::LibraryLoaded);

  base::android::InitVM(vm);
  JNIEnv* env = base::android::AttachCurrentThread();
  if (!base::android::RegisterLibraryLoaderEntryHook(env))
    return -1;

  if (!xwalk::RegisterJni(env))
    return -1;

  #if defined(USE_COCOS2D)
  if (!cocos2d::RegisterCocos2dJni(env))
    return -1;
  #endif

  content::SetContentMainDelegate(new xwalk::XWalkMainDelegateAndroid());

  return JNI_VERSION_1_4;
}
