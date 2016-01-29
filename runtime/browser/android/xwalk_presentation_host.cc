// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_presentation_host.h"

#include "base/android/jni_array.h"
#include "base/android/jni_string.h"
#include "base/android/locale_utils.h"
#include "base/base_paths_android.h"
#include "content/public/browser/browser_thread.h"

#include "jni/XWalkPresentationHost_jni.h"

using base::android::AttachCurrentThread;

namespace xwalk {

XWalkPresentationHost* XWalkPresentationHost::gInstance = nullptr;

XWalkPresentationHost* XWalkPresentationHost::GetInstance() {
  return XWalkPresentationHost::gInstance;
}

XWalkPresentationHost::XWalkPresentationHost(JNIEnv* env, jobject obj)
  : java_ref_(env, obj),
  display_change_callback_(nullptr) {
}

XWalkPresentationHost::~XWalkPresentationHost() {
}

void XWalkPresentationHost::Destroy(JNIEnv* env, jobject obj) {
  delete this;
}

void XWalkPresentationHost::SetupJavaPeer(JNIEnv* env, jobject obj) {
  gInstance = this;
}

void XWalkPresentationHost::OnDisplayAdded(JNIEnv* env,
    jobject obj, int display_d) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

  if ( display_change_callback_ ) {
    display_change_callback_(display_d);
  }
}

void XWalkPresentationHost::OnDisplayChanged(JNIEnv* env,
    jobject obj, int display_d) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

  if ( display_change_callback_ ) {
    display_change_callback_(display_d);
  }
}

void XWalkPresentationHost::OnDisplayRemoved(JNIEnv* env,
    jobject obj, int display_d) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

  if ( display_change_callback_ ) {
    display_change_callback_(display_d);
  }
}

void XWalkPresentationHost::OnPresentationClosed(JNIEnv* env, jobject obj,
    const int render_process_id, const int render_frame_id) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

  FOR_EACH_OBSERVER(SessionObserver, observers_,
    OnPresentationClosed(render_process_id, render_frame_id));
}

std::vector<XWalkPresentationHost::AndroidDisplay> XWalkPresentationHost::
    GetAndroidDisplayInfo() const {
  std::vector<XWalkPresentationHost::AndroidDisplay> result;

  JNIEnv* javaEnv = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = java_ref_.get(javaEnv);
  if ( obj.is_null() ) {
    LOG(ERROR) << "Failed to obtain JNI object";
    return result;  // an empty vector
  }

  auto jObjectArray =
    Java_XWalkPresentationHost_getAndroidDisplayInfo(javaEnv, obj.obj());
  int COUNT = javaEnv->GetArrayLength(jObjectArray.obj());

  for ( int i = 0 ; i < COUNT ; ++i ) {
    auto object = javaEnv->GetObjectArrayElement(jObjectArray.obj(), i);
    auto jclass = javaEnv->GetObjectClass(object);

    std::string cxxName("");
    int cxxID = -1;
    int cxxFlags = 0;

    // Extract name of the display
    auto nameMethod = javaEnv->GetMethodID(jclass,
      "getName", "()Ljava/lang/String;");
    if ( nameMethod ) {
      auto value = (jstring)javaEnv->CallObjectMethod(object, nameMethod);
      base::android::ConvertJavaStringToUTF8(javaEnv, value, &cxxName);
    }

    // Extract ID of the display
    auto getIDMethod = javaEnv->GetMethodID(jclass, "getDisplayId", "()I");
    if ( getIDMethod ) {
      auto value = (jint)javaEnv->CallIntMethod(object, getIDMethod);
      cxxID = value;
    }

    // Extract Flags of the display
    auto getFlagsMethod = javaEnv->GetMethodID(jclass, "getFlags", "()I");
    if ( getFlagsMethod ) {
      auto value = (jint)javaEnv->CallIntMethod(object, getFlagsMethod);
      cxxFlags = value;
    }

    // const unsigned int ANDROID_JAVA_FLAG_ROUND = 0x10;
    const unsigned int ANDROID_JAVA_FLAG_PRESENTATION = 0x8;
    // const unsigned int ANDROID_JAVA_FLAG_PRIVATE = 0x4;
    // const unsigned int ANDROID_JAVA_FLAG_SECURE = 0x2;
    // const unsigned int ANDROID_JAVA_FLAG_SUPPORTS_PROTECTED_BUFFERS = 0x1;

    XWalkPresentationHost::AndroidDisplay display;
    display.name = cxxName;
    display.id = cxxID;
    display.is_presentation = (cxxFlags & ANDROID_JAVA_FLAG_PRESENTATION) != 0;
    display.is_primary = !display.is_presentation;

    result.push_back(display);
  }

  return result;
}

bool XWalkPresentationHost::ShowPresentation(const int render_process_id,
    const int render_frame_id,
    const int display_d, const std::string& url) {
  bool success = false;

  JNIEnv* javaEnv = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = java_ref_.get(javaEnv);
  if ( obj.is_null() ) {
    LOG(ERROR) << "Failed to obtain JNI object";
    return success;
  }

  ScopedJavaLocalRef<jstring> javaString =
    base::android::ConvertUTF8ToJavaString(javaEnv, url);
  success = Java_XWalkPresentationHost_showPresentation(javaEnv, obj.obj(),
    render_process_id, render_frame_id, display_d, javaString.obj());

  return success;
}

void XWalkPresentationHost::closePresentation(const int render_process_id,
    const int render_frame_id) {
  JNIEnv* javaEnv = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = java_ref_.get(javaEnv);
  if ( obj.is_null() ) {
    LOG(ERROR) << "Failed to obtain JNI object";
  }

  Java_XWalkPresentationHost_closePresentation(javaEnv, obj.obj(),
    render_process_id, render_frame_id);
}

static jlong Init(JNIEnv* env, const JavaParamRef<jobject>& obj) {
  auto nativeObj = new XWalkPresentationHost(env, obj);
  return reinterpret_cast<intptr_t>(nativeObj);
}

bool RegisterXWalkPresentationHost(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace xwalk
