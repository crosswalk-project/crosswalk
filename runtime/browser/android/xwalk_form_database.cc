// Copyright 2016 The Intel Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_form_database.h"

#include "base/android/jni_android.h"

#include "xwalk/runtime/browser/xwalk_browser_context.h"
#include "jni/XWalkFormDatabase_jni.h"

namespace xwalk {

namespace {

XWalkFormDatabaseService* GetFormDatabaseService() {

  XWalkBrowserContext* context = XWalkBrowserContext::GetDefault();
  XWalkFormDatabaseService* service = context->GetFormDatabaseService();
  return service;
}

} // anonymous namespace

// static
jboolean HasFormData(JNIEnv*, const JavaParamRef<jclass>&) {
  return GetFormDatabaseService()->HasFormData();
}

// static
void ClearFormData(JNIEnv*, const JavaParamRef<jclass>&) {
  GetFormDatabaseService()->ClearFormData();
}

bool RegisterXWalkFormDatabase(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

} // namespace xwalk
