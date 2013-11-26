// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/android/xwalk_extension_android.h"

#include "base/android/jni_android.h"
#include "base/bind.h"
#include "base/logging.h"
#include "jni/XWalkExtensionAndroid_jni.h"
#include "xwalk/extensions/common/xwalk_extension.h"
#include "xwalk/runtime/browser/xwalk_browser_main_parts_android.h"
#include "xwalk/runtime/browser/xwalk_content_browser_client.h"

namespace xwalk {
namespace extensions {

XWalkExtensionAndroid::XWalkExtensionAndroid(JNIEnv* env, jobject obj,
                                             jstring name, jstring js_api)
    : XWalkExtension(),
      java_ref_(env, obj),
      next_instance_id_(1) {
  const char *str = env->GetStringUTFChars(name, 0);
  set_name(str);
  env->ReleaseStringUTFChars(name, str);

  str = env->GetStringUTFChars(js_api, 0);
  set_javascript_api(str);
  env->ReleaseStringUTFChars(js_api, str);
}

XWalkExtensionAndroid::~XWalkExtensionAndroid() {
  //  Clear its all instances.
  for (InstanceMap::iterator it = instances_.begin();
       it != instances_.end(); ++it) {
    XWalkExtensionInstance* instance = it->second;
    delete instance;
  }
  instances_.clear();
}

bool XWalkExtensionAndroid::is_valid() {
  if (instances_.empty() || javascript_api().empty()) {
    return false;
  }

  return true;
}

void XWalkExtensionAndroid::PostMessage(JNIEnv* env, jobject obj,
                                       jint instance, jstring msg) {
  if (!is_valid()) return;

  InstanceMap::iterator it = instances_.find(instance);
  if (it == instances_.end()) {
    LOG(WARNING) << "Instance(" << instance << ") not found ";
    return;
  }

  const char* str = env->GetStringUTFChars(msg, 0);
  it->second->PostMessageWrapper(str);
  env->ReleaseStringUTFChars(msg, str);
}

void XWalkExtensionAndroid::BroadcastMessage(JNIEnv* env, jobject obj,
                                             jstring msg) {
  if (!is_valid()) return;

  const char* str = env->GetStringUTFChars(msg, 0);
  for (InstanceMap::iterator it = instances_.begin();
       it != instances_.end(); ++it) {
    it->second->PostMessageWrapper(str);
  }
  env->ReleaseStringUTFChars(msg, str);
}

void XWalkExtensionAndroid::DestroyExtension(JNIEnv* env, jobject obj) {
    // Instead of using 'delete' to destroy the extension object, here we
    // reply on the method of UnregisterExtension to delete it since it will
    // remove a scoped_ptr of XWalkExtensionAndroid object from a
    // ScopedVector object. Otherwise, the 'delete' operation would be
    // called recursively.
    ToAndroidMainParts(XWalkContentBrowserClient::Get()->main_parts())->
        UnregisterExtension(scoped_ptr<XWalkExtension>(this));
}

XWalkExtensionInstance* XWalkExtensionAndroid::CreateInstance() {
  JNIEnv* env = base::android::AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null()) {
    return NULL;
  }

  XWalkExtensionAndroidInstance* instance =
      new XWalkExtensionAndroidInstance(this, java_ref_, next_instance_id_);
  instances_[next_instance_id_] = instance;

  next_instance_id_++;
  return instance;
}

void XWalkExtensionAndroid::RemoveInstance(int instance) {
  JNIEnv* env = base::android::AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null()) {
    return;
  }

  InstanceMap::iterator it = instances_.find(instance);
  if (it == instances_.end()) {
    LOG(WARNING) << "Instance(" << instance << ") not found ";
    return;
  }

  instances_.erase(instance);
}

XWalkExtensionAndroidInstance::XWalkExtensionAndroidInstance(
    XWalkExtensionAndroid* extension,
    const JavaObjectWeakGlobalRef& java_ref,
    int id)
    : extension_(extension),
      java_ref_(java_ref),
      id_(id) {
}

XWalkExtensionAndroidInstance::~XWalkExtensionAndroidInstance() {
  extension_->RemoveInstance(id_);
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

  Java_XWalkExtensionAndroid_handleMessage(env, obj.obj(), getID(), buffer);
}

void XWalkExtensionAndroidInstance::HandleSyncMessage(
    scoped_ptr<base::Value> msg) {
  base::StringValue* ret_val = base::Value::CreateStringValue("");

  std::string value;
  if (!msg->GetAsString(&value)) {
    SendSyncReplyToJS(scoped_ptr<base::Value>(ret_val));
    return;
  }

  JNIEnv* env = base::android::AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null()) {
    SendSyncReplyToJS(scoped_ptr<base::Value>(ret_val));
    return;
  }

  jstring buffer = env->NewStringUTF(value.c_str());
  ScopedJavaLocalRef<jstring> ret =
      Java_XWalkExtensionAndroid_handleSyncMessage(
              env, obj.obj(), getID(), buffer);

  const char *str = env->GetStringUTFChars(ret.obj(), 0);
  ret_val = base::Value::CreateStringValue(str);
  env->ReleaseStringUTFChars(ret.obj(), str);

  SendSyncReplyToJS(scoped_ptr<base::Value>(ret_val));
}

static jint CreateExtension(JNIEnv* env, jobject obj,
                            jstring name, jstring js_api) {
  XWalkExtensionAndroid* extension =
      new XWalkExtensionAndroid(env, obj, name, js_api);

  ToAndroidMainParts(XWalkContentBrowserClient::Get()->main_parts())->
      RegisterExtension(scoped_ptr<XWalkExtension>(extension));

  return reinterpret_cast<jint>(extension);
}

bool RegisterXWalkExtensionAndroid(JNIEnv* env) {
  return RegisterNativesImpl(env) >= 0;
}

}  // namespace extensions
}  // namespace xwalk
