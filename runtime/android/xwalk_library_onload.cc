// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/basictypes.h"
#include "base/debug/debugger.h"
#include "base/logging.h"
#include "base/android/jni_android.h"
#include "base/android/jni_registrar.h"
#include "content/public/app/android_library_loader_hooks.h"
#include "content/public/app/content_main.h"
#include "content/public/browser/android/compositor.h"

#include "xwalk/runtime/app/xwalk_main_delegate.h"
#include "xwalk/runtime/android/xwalk_jni_registrar.h"

// This is called by the VM when the shared library is first loaded.
JNI_EXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  base::android::InitVM(vm);
  JNIEnv* env = base::android::AttachCurrentThread();

  if (!content::RegisterLibraryLoaderEntryHook(env))
    return -1;

  // To be called only from the UI thread.  If loading the library is done on
  // a separate thread, this should be moved elsewhere.
  if (!xwalk::android::RegisterXWalkJni(env))
    return -1;

  content::Compositor::Initialize();

  // The delegate to use when the android activity calls ContentMain
  content::SetContentMainDelegate(new xwalk::XWalkMainDelegate());

  return JNI_VERSION_1_4;
}

