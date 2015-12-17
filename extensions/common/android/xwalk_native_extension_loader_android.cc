// Copyright (c) 2015 Intel Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/android/xwalk_native_extension_loader_android.h"

#include <string>

#include "jni/XWalkNativeExtensionLoaderAndroid_jni.h"
#include "xwalk/runtime/browser/xwalk_browser_main_parts_android.h"
#include "xwalk/runtime/browser/xwalk_content_browser_client.h"

namespace xwalk {
namespace extensions {

void RegisterExtensionInPath(JNIEnv* env,
                             const JavaParamRef<jclass>& jcaller,
                             const JavaParamRef<jstring>& path) {
  const char *str = env->GetStringUTFChars(path, 0);
  xwalk::XWalkBrowserMainPartsAndroid* main_parts =
      ToAndroidMainParts(XWalkContentBrowserClient::Get()->main_parts());
  main_parts->RegisterExtensionInPath(str);
  env->ReleaseStringUTFChars(path, str);
}

bool RegisterXWalkNativeExtensionLoaderAndroid(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace extensions
}  // namespace xwalk
