// Copyright (c) 2014 Intel Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_view_delegate.h"

#include "base/android/jni_android.h"
#include "build/build_config.h"
#include "jni/XWalkViewDelegate_jni.h"

namespace xwalk {

jboolean IsLibraryBuiltForIA(JNIEnv* env, jclass jcaller) {
#if defined(ARCH_CPU_X86)
  return JNI_TRUE;
#else
  return JNI_FALSE;
#endif
}

bool RegisterXWalkViewDelegate(JNIEnv* env) {
  return RegisterNativesImpl(env) >= 0;
}

}  // namespace xwalk
