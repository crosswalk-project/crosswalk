// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_form_database.h"

#include "base/android/jni_android.h"
#include "base/logging.h"
#include "base/time/time.h"
#include "components/autofill/core/browser/webdata/autofill_webdata_service.h"
#include "jni/XWalkFormDatabase_jni.h"
#include "xwalk/runtime/browser/android/xwalk_content.h"
#include "xwalk/runtime/browser/android/xwalk_form_database_service.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/xwalk_content_browser_client.h"



// static
scoped_refptr<autofill::AutofillWebDataService>
autofill::AutofillWebDataService::FromBrowserContext(
    content::BrowserContext* context) {

  DCHECK(context);
  xwalk::XWalkFormDatabaseService* service =
      static_cast<xwalk::RuntimeContext*>(
          context)->GetFormDatabaseService();
  DCHECK(service);
  return service->get_autofill_webdata_service();
}

namespace xwalk {

namespace {

XWalkFormDatabaseService* GetFormDatabaseService() {
  RuntimeContext* context = XWalkContentBrowserClient::GetRuntimeContext();
  XWalkFormDatabaseService* service = context->GetFormDatabaseService();
  return service;
}

}  // anonymous namespace


// static
jboolean HasFormData(JNIEnv*, jclass) {
  return GetFormDatabaseService()->HasFormData();
}

// static
void ClearFormData(JNIEnv*, jclass) {
  GetFormDatabaseService()->ClearFormData();
}

bool RegisterXWalkFormDatabase(JNIEnv* env) {
  return RegisterNativesImpl(env) >= 0;
}

}  // namespace xwalk
