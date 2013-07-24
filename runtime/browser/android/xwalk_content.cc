// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_content.h"

#include "base/base_paths_android.h"
#include "base/path_service.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/browser_context.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/xwalk_content_browser_client.h"
#include "jni/XWalkContent_jni.h"

namespace xwalk {

XWalkContent::XWalkContent(JNIEnv* env, jobject obj)
    : java_ref_(env, obj) {
}

XWalkContent::~XWalkContent() {
}

jint XWalkContent::GetWebContents(JNIEnv* env, jobject obj) {
  if (!web_contents_) {
    web_contents_.reset(CreateWebContents());
  }
  return reinterpret_cast<jint>(web_contents_.get());
}

content::WebContents* XWalkContent::CreateWebContents() {
  RuntimeContext* runtime_context =
      XWalkContentBrowserClient::GetRuntimeContext();
  return content::WebContents::Create(
      content::WebContents::CreateParams(runtime_context));
}

void XWalkContent::Destroy(JNIEnv* env, jobject obj) {
  delete this;
}

static jint Init(JNIEnv* env, jobject obj) {
  XWalkContent* xwalk_core_content = new XWalkContent(env, obj);
  return reinterpret_cast<jint>(xwalk_core_content);
}

bool RegisterXWalkContent(JNIEnv* env) {
  return RegisterNativesImpl(env) >= 0;
}

}  // namespace xwalk
