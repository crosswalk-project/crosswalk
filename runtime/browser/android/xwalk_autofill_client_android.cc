// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_autofill_client_android.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "components/autofill/core/browser/autofill_manager.h"
#include "jni/XWalkAutofillClientAndroid_jni.h"
#include "ui/gfx/geometry/rect_f.h"
#include "xwalk/runtime/browser/android/xwalk_content.h"

using base::android::AttachCurrentThread;
using base::android::ConvertUTF16ToJavaString;
using base::android::ScopedJavaLocalRef;
using content::WebContents;

DEFINE_WEB_CONTENTS_USER_DATA_KEY(xwalk::XWalkAutofillClientAndroid);

namespace xwalk {

// Ownership: The native object is created (if autofill enabled) and owned by
// XWalkContent. The native object creates the java peer which handles most
// autofill functionality at the java side. The java peer is owned by Java
// XWalkContent. The native object only maintains a weak ref to it.
XWalkAutofillClientAndroid::XWalkAutofillClientAndroid(WebContents* contents)
  : XWalkAutofillClient(contents) {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> delegate;
  delegate.Reset(
      Java_XWalkAutofillClientAndroid_create(
          env,
          reinterpret_cast<intptr_t>(this)));
  XWalkContent* xwalk_content = XWalkContent::FromWebContents(contents);
  xwalk_content->SetXWalkAutofillClient(delegate.obj());
  java_ref_ = JavaObjectWeakGlobalRef(env, delegate.obj());
}

void XWalkAutofillClientAndroid::ShowAutofillPopupImpl(
    const gfx::RectF& element_bounds,
    base::i18n::TextDirection text_direction,
    const std::vector<autofill::Suggestion>& suggestions) {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null()) return;

  // We need an array of AutofillSuggestion.
  size_t count = suggestions.size();

  ScopedJavaLocalRef<jobjectArray> data_array =
      Java_XWalkAutofillClientAndroid_createAutofillSuggestionArray(env, count);

  for (size_t i = 0; i < count; ++i) {
    ScopedJavaLocalRef<jstring> name =
        ConvertUTF16ToJavaString(env, suggestions[i].value);
    ScopedJavaLocalRef<jstring> label =
        ConvertUTF16ToJavaString(env, suggestions[i].label);
    Java_XWalkAutofillClientAndroid_addToAutofillSuggestionArray(
        env, data_array.obj(), i, name.obj(), label.obj(),
        suggestions[i].frontend_id);
  }

  Java_XWalkAutofillClientAndroid_showAutofillPopup(
      env,
      obj.obj(),
      element_bounds.x(),
      element_bounds.y(),
      element_bounds.width(),
      element_bounds.height(),
      text_direction == base::i18n::RIGHT_TO_LEFT,
      data_array.obj());
}

void XWalkAutofillClientAndroid::HideAutofillPopupImpl() {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null())
    return;
  Java_XWalkAutofillClientAndroid_hideAutofillPopup(env, obj.obj());
}

void XWalkAutofillClientAndroid::SuggestionSelected(JNIEnv* env,
                                                    jobject object,
                                                    jint position) {
  XWalkAutofillClient::SuggestionSelected(position);
}

bool RegisterXWalkAutofillClient(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace xwalk
