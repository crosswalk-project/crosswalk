// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/app/android/xwalk_jni_registrar.h"

#include "base/android/jni_android.h"
#include "base/android/jni_registrar.h"
#include "components/navigation_interception/component_jni_registrar.h"
#include "components/web_contents_delegate_android/component_jni_registrar.h"
#include "xwalk/extensions/common/android/xwalk_extension_android.h"
#include "xwalk/runtime/browser/android/net/android_protocol_handler.h"
#include "xwalk/runtime/browser/android/net/input_stream_impl.h"
#include "xwalk/runtime/browser/android/xwalk_content.h"
#include "xwalk/runtime/browser/android/xwalk_contents_client_bridge.h"
#include "xwalk/runtime/browser/android/xwalk_dev_tools_server.h"
#include "xwalk/runtime/browser/android/xwalk_settings.h"
#include "xwalk/runtime/browser/android/xwalk_web_contents_delegate.h"

namespace xwalk {

static base::android::RegistrationMethod kXWalkRegisteredMethods[] = {
  // Register JNI for xwalk classes.
  { "AndroidProtocolHandler", RegisterAndroidProtocolHandler },
  { "InputStream", RegisterInputStream },
  { "NavigationInterception",
      navigation_interception::RegisterNavigationInterceptionJni },
  { "WebContentsDelegateAndroid",
      web_contents_delegate_android::RegisterWebContentsDelegateAndroidJni },
  { "XWalkContentsClientBridge", RegisterXWalkContentsClientBridge },
  { "XWalkContent", RegisterXWalkContent },
  { "XWalkDevToolsServer", RegisterXWalkDevToolsServer },
  { "XWalkExtensionAndroid", extensions::RegisterXWalkExtensionAndroid },
  { "XWalkSettings", RegisterXWalkSettings },
  { "XWalkWebContentsDelegate", RegisterXWalkWebContentsDelegate },
};

bool RegisterJni(JNIEnv* env) {
  return RegisterNativeMethods(env,
    kXWalkRegisteredMethods, arraysize(kXWalkRegisteredMethods));
}

}  // namespace xwalk
