// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_COMMON_ANDROID_XWALK_EXTENSION_ANDROID_H_
#define XWALK_EXTENSIONS_COMMON_ANDROID_XWALK_EXTENSION_ANDROID_H_

#include <map>
#include <string>

#include "base/android/jni_helper.h"
#include "base/android/scoped_java_ref.h"
#include "base/callback.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension.h"

namespace xwalk {
namespace extensions {

class XWalkExtensionAndroidInstance;

// This class (together with XWalkExtensionAndroidInstance) is counterpart of
// XWalkExtensionAndroid.java, allow implement message passing based extension
// written by Java.
//
// When new a Java object that inherit from XWalkExtensionAndroid.java, it will
// create a XWalkExtensionAndroid via the JNI interface CreateExtension.
// Then C++ extension framework will create XWalkExtensionInstance(s) on the
// fly. Thus instance(s) can bridge messages between JS and Java.
class XWalkExtensionAndroid : public XWalkExtension {
 public:
  XWalkExtensionAndroid(JNIEnv* env, jobject obj, jstring name, jstring js_api);
  virtual ~XWalkExtensionAndroid();

  // JNI interface to post message from Java to JS
  void PostMessage(JNIEnv* env, jobject obj, jint instance, jstring msg);

  virtual const char* GetJavaScriptAPI() OVERRIDE;
  virtual XWalkExtensionInstance* CreateInstance(
      const XWalkExtension::PostMessageCallback& post_message) OVERRIDE;

  void RemoveInstance(int instance);

 private:
  bool is_valid();

  typedef std::map<int, XWalkExtensionAndroidInstance*> InstanceMap;
  InstanceMap instances_;
  JavaObjectWeakGlobalRef java_ref_;
  std::string js_api_;
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
    PostMessageToJS(scoped_ptr<base::Value>(new base::StringValue(msg)));
  }

 private:
  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE;
  virtual scoped_ptr<base::Value> HandleSyncMessage(
      scoped_ptr<base::Value> msg) OVERRIDE;

  XWalkExtensionAndroid* extension_;
  JavaObjectWeakGlobalRef java_ref_;
  int id_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionAndroidInstance);
};

bool RegisterXWalkExtensionAndroid(JNIEnv* env);

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_COMMON_ANDROID_XWALK_EXTENSION_ANDROID_H_
