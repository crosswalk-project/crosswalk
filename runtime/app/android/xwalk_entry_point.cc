// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/android/jni_android.h"
#include "base/bind.h"
#include "content/public/app/content_jni_onload.h"
#include "content/public/app/content_main.h"
#include "content/public/app/content_main_delegate.h"
#include "xwalk/runtime/app/android/xwalk_jni_registrar.h"
#include "xwalk/runtime/app/android/xwalk_main_delegate_android.h"

namespace {

bool RegisterJNI(JNIEnv* env) {
  return xwalk::RegisterJni(env);
}

bool Init() {
  content::SetContentMainDelegate(new xwalk::XWalkMainDelegateAndroid());
  return true;
}

}  // namespace

// This is called by the VM when the shared library is first loaded.
JNI_EXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  std::vector<base::android::RegisterCallback> register_callbacks;
  register_callbacks.push_back(base::Bind(&RegisterJNI));

  std::vector<base::android::InitCallback> init_callbacks;
  init_callbacks.push_back(base::Bind(&Init));

  if (!content::android::OnJNIOnLoadRegisterJNI(
          vm, register_callbacks) ||
      !content::android::OnJNIOnLoadInit(init_callbacks))
    return -1;
  return JNI_VERSION_1_4;
}
