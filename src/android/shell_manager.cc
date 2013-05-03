// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/android/shell_manager.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "base/bind.h"
#include "base/lazy_instance.h"
#include "cameo/src/shell.h"
#include "cameo/src/shell_browser_context.h"
#include "cameo/src/shell_content_browser_client.h"
#include "cameo/src/shell.h"
#include "content/public/browser/web_contents.h"
#include "googleurl/src/gurl.h"
#include "jni/ShellManager_jni.h"

using base::android::ScopedJavaLocalRef;

namespace {

struct GlobalState {
  GlobalState() {}
  base::android::ScopedJavaGlobalRef<jobject> j_shell_manager;
};

base::LazyInstance<GlobalState> g_global_state = LAZY_INSTANCE_INITIALIZER;

}  // namespace

namespace content {

jobject CreateShellView(Shell* shell) {
  JNIEnv* env = base::android::AttachCurrentThread();
  jobject j_shell_manager = g_global_state.Get().j_shell_manager.obj();
  return Java_ShellManager_createShell(env, j_shell_manager).Release();
}

// Register native methods
bool RegisterShellManager(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

static void Init(JNIEnv* env, jclass clazz, jobject obj) {
  g_global_state.Get().j_shell_manager.Reset(
      base::android::ScopedJavaLocalRef<jobject>(env, obj));
}

void LaunchShell(JNIEnv* env, jclass clazz, jstring jurl) {
  ShellBrowserContext* browserContext =
      ShellContentBrowserClient::Get()->browser_context();
  GURL url(base::android::ConvertJavaStringToUTF8(env, jurl));
  Shell::CreateNewWindow(browserContext,
                         url,
                         NULL,
                         MSG_ROUTING_NONE,
                         gfx::Size());
}

}  // namespace content
