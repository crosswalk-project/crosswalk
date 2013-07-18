// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/android/xwalk_view.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "base/bind.h"
#include "base/lazy_instance.h"
#include "content/shell/shell.h"
#include "content/shell/shell_browser_context.h"
#include "content/shell/shell_content_browser_client.h"
#include "content/public/browser/web_contents.h"
#include "jni/XWalkView_jni.h"

using base::android::ScopedJavaLocalRef;

namespace {

struct GlobalState {
  GlobalState() {}
  base::android::ScopedJavaGlobalRef<jobject> j_shell_manager;
};

base::LazyInstance<GlobalState> g_global_state = LAZY_INSTANCE_INITIALIZER;

}  // namespace

namespace xwalk {

bool RegisterXWalkView(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

void InitWithWebContents(content::WebContents *web_contents) {
  JNIEnv *env = base::android::AttachCurrentThread();
  Java_XWalkView_initWithWebContents(env, g_global_state.Get().j_shell_manager.obj(), reinterpret_cast<jint>(web_contents));
}

static void Init(JNIEnv* env, jclass clazz, jobject obj) {
  g_global_state.Get().j_shell_manager.Reset(
      base::android::ScopedJavaLocalRef<jobject>(env, obj));
}

}   // namespace xwalk
