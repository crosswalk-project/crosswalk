// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_COMMON_ANDROID_XWALK_EXTENSION_ANDROID_H_
#define XWALK_EXTENSIONS_COMMON_ANDROID_XWALK_EXTENSION_ANDROID_H_

#include <map>
#include <memory>
#include <string>

#include "base/android/jni_weak_ref.h"
#include "base/android/scoped_java_ref.h"
#include "base/callback.h"
#include "base/logging.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension.h"

namespace xwalk {
namespace extensions {

class XWalkExtensionAndroidInstance;

// This class (together with XWalkExtensionAndroidInstance) is the native part
// of XWalkExtensionAndroid on Java side for message-passing based extension
// system works on Android platform.
//
// Once a XWalkExtensionAndroid-derived instance is created on Java side, the
// native part is also created accordingly. Meanwhile, the native part has to
// hold a reference to the Java-side object for routing message from native side
// to Java side.
//
// The native-side XWalkExtensionAndroid/XWalkExtensionInstance objects are
// actually owned by XWalkExtensionServer, it means that XWalkExtensionServer
// is the right place to delete those objects. The native XWalkExtensionAndroid
// object is always alive in memory until the process is terminated after it is
// registered into extension server.
//
// The Java-side object may be destroyed due to the lifecycle of Activity, e.g.
// when pressing back button, the Activity will invoke onDestroyed callback and
// hence Java-side objects are treated to be invalid.
//
// Since the native part of XWalkExtensionAndroid is designed to be reused
// during the whole process lifecycle. For each native part, its referenced
// Java-side object needs to be re-assigned once a new Java extension object is
// created for the same extension identified the extension name.
class XWalkExtensionAndroid : public XWalkExtension {
 public:
  XWalkExtensionAndroid(JNIEnv* env, jobject obj, jstring name,
                        jstring js_api, jobjectArray js_entry_ports);
  ~XWalkExtensionAndroid() override;

  // JNI interface to post message from Java to JS
  void PostMessage(JNIEnv* env, jobject obj, jint instance, jstring msg);
  void PostBinaryMessage(JNIEnv* env, jobject obj, jint instance,
                         jbyteArray msg);
  void BroadcastMessage(JNIEnv* env, jobject obj, jstring msg);

  void DestroyExtension(JNIEnv* env, jobject obj);

  XWalkExtensionInstance* CreateInstance() override;

  void RemoveInstance(int instance);

  // Each Extension object created on Java side is backed by this native object,
  // and the native object also has a reference to Java-side object for message
  // routing from native side to Java side. However, the Java extension object
  // might be destroyed due to activity lifecycle. This method is used to
  // re-bind the native object to an valid Java-side extension object.
  void BindToJavaObject(JNIEnv* env, jobject obj);

 private:
  bool is_valid();

  typedef std::map<int, XWalkExtensionAndroidInstance*> InstanceMap;
  InstanceMap instances_;
  // Hold a reference to Java-side extension object for message routing.
  JavaObjectWeakGlobalRef java_ref_;
  int next_instance_id_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionAndroid);
};

class XWalkExtensionAndroidInstance : public XWalkExtensionInstance {
 public:
  explicit XWalkExtensionAndroidInstance(
      XWalkExtensionAndroid* extension,
      const JavaObjectWeakGlobalRef& java_ref,
      int id);
  ~XWalkExtensionAndroidInstance();

  void PostMessageWrapper(const char* msg) {
    PostMessageToJS(std::unique_ptr<base::Value>(new base::StringValue(msg)));
  }
  void PostBinaryMessageWrapper(const char* msg, const size_t size) {
    PostMessageToJS(std::unique_ptr<base::Value>(
        base::BinaryValue::CreateWithCopiedBuffer(msg, size)));
  }

  int getID() {
      return id_;
  }

 private:
  void HandleMessage(std::unique_ptr<base::Value> msg) override;
  void HandleSyncMessage(std::unique_ptr<base::Value> msg) override;

  XWalkExtensionAndroid* extension_;
  // Hold a refenerence to Java-side XWalkExtensionAndroid object.
  JavaObjectWeakGlobalRef java_ref_;
  int id_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionAndroidInstance);
};

bool RegisterXWalkExtensionAndroid(JNIEnv* env);

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_COMMON_ANDROID_XWALK_EXTENSION_ANDROID_H_
