// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/android/xwalk_extension_android.h"

#include "base/android/jni_android.h"
#include "base/bind.h"
#include "base/logging.h"
#include "jni/XWalkExtensionAndroid_jni.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/runtime/browser/xwalk_browser_main_parts.h"
#include "xwalk/runtime/browser/xwalk_content_browser_client.h"

namespace xwalk {
namespace extensions {

XWalkExtensionAndroid::XWalkExtensionAndroid(JNIEnv* env, jobject obj,
                                             jstring name, jstring js_api)
    : XWalkExtension(),
      instance_(NULL),
      java_ref_(env, obj) {
  const char *str = env->GetStringUTFChars(name, 0);
  set_name(str);
  env->ReleaseStringUTFChars(name, str);

  str = env->GetStringUTFChars(js_api, 0);
  js_api_ = str;
  env->ReleaseStringUTFChars(js_api, str);
}

XWalkExtensionAndroid::~XWalkExtensionAndroid() {
  JNIEnv* env = base::android::AttachCurrentThread();

  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null()) {
    return;
  }

  Java_XWalkExtensionAndroid_onDestroy(env, obj.obj());
}

bool XWalkExtensionAndroid::is_valid() {
  if (!instance_) {
    return false;
  }

  return true;
}

void XWalkExtensionAndroid::PostMessage(JNIEnv* env, jobject obj, jstring msg) {
  DCHECK(is_valid());

  const char* str = env->GetStringUTFChars(msg, 0);
  instance_->PostMessageWrapper(str);
  env->ReleaseStringUTFChars(msg, str);
}

const char* XWalkExtensionAndroid::GetJavaScriptAPI() {
  return js_api_.c_str();
}

XWalkExtensionInstance* XWalkExtensionAndroid::CreateInstance(
    const XWalkExtension::PostMessageCallback& post_message) {
  if (!instance_) {
    instance_ = new XWalkExtensionAndroidInstance(java_ref_);
    instance_->SetPostMessageCallback(post_message);
  }

  return instance_;
}

XWalkExtensionAndroidInstance::XWalkExtensionAndroidInstance(
    const JavaObjectWeakGlobalRef& java_ref)
    : java_ref_(java_ref) {
}

XWalkExtensionAndroidInstance::~XWalkExtensionAndroidInstance() {
}

void XWalkExtensionAndroidInstance::HandleMessage(
    scoped_ptr<base::Value> msg) {
  std::string value;

  if (!msg->GetAsString(&value)) {
    return;
  }

  JNIEnv* env = base::android::AttachCurrentThread();
  jstring buffer = env->NewStringUTF(value.c_str());
  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null()) {
    return;
  }

  Java_XWalkExtensionAndroid_handleMessage(env, obj.obj(), buffer);
}

scoped_ptr<base::Value>
XWalkExtensionAndroidInstance::HandleSyncMessage(
    scoped_ptr<base::Value> msg) {
  base::StringValue* ret_val = base::Value::CreateStringValue("");

  std::string value;
  if (!msg->GetAsString(&value)) {
    return scoped_ptr<base::Value>(ret_val);
  }

  JNIEnv* env = base::android::AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null()) {
    return scoped_ptr<base::Value>(ret_val);
  }

  jstring buffer = env->NewStringUTF(value.c_str());
  ScopedJavaLocalRef<jstring> ret =
      Java_XWalkExtensionAndroid_handleSyncMessage(env, obj.obj(), buffer);

  const char *str = env->GetStringUTFChars(ret.obj(), 0);
  ret_val = base::Value::CreateStringValue(str);
  env->ReleaseStringUTFChars(ret.obj(), str);

  return scoped_ptr<base::Value>(ret_val);
}

static jint CreateExtension(JNIEnv* env, jobject obj,
                            jstring name, jstring js_api) {
  XWalkExtensionAndroid* extension =
      new XWalkExtensionAndroid(env, obj, name, js_api);
  XWalkExtensionService* service =
      XWalkContentBrowserClient::Get()->main_parts()->extension_service();
  service->RegisterExtension(scoped_ptr<XWalkExtension>(extension));
  return reinterpret_cast<jint>(extension);
}

bool RegisterXWalkExtensionAndroid(JNIEnv* env) {
  return RegisterNativesImpl(env) >= 0;
}

}  // namespace extensions
}  // namespace xwalk
