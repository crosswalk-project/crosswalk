// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/app/android/xwalk_jni_registrar.h"

#include "base/android/jni_android.h"
#include "base/android/jni_registrar.h"
#include "components/web_contents_delegate_android/component_jni_registrar.h"
#include "net/android/net_jni_registrar.h"
#include "xwalk/extensions/common/android/xwalk_extension_android.h"
#include "xwalk/extensions/common/android/xwalk_native_extension_loader_android.h"
#include "xwalk/runtime/browser/android/cookie_manager.h"
#include "xwalk/runtime/browser/android/net/android_protocol_handler.h"
#include "xwalk/runtime/browser/android/xwalk_autofill_client_android.h"
#include "xwalk/runtime/browser/android/xwalk_content.h"
#include "xwalk/runtime/browser/android/xwalk_contents_client_bridge.h"
#include "xwalk/runtime/browser/android/xwalk_dev_tools_server.h"
#include "xwalk/runtime/browser/android/xwalk_http_auth_handler.h"
#include "xwalk/runtime/browser/android/xwalk_path_helper.h"
#include "xwalk/runtime/browser/android/xwalk_presentation_host.h"
#include "xwalk/runtime/browser/android/xwalk_settings.h"
#include "xwalk/runtime/browser/android/xwalk_view_delegate.h"

namespace xwalk {

static base::android::RegistrationMethod kXWalkRegisteredMethods[] = {
  // Register JNI for xwalk classes.
  { "AndroidProtocolHandler", RegisterAndroidProtocolHandler },
  { "CookieManager", RegisterCookieManager },
  { "NetAndroid", net::android::RegisterJni },
  { "WebContentsDelegateAndroid",
      web_contents_delegate_android::RegisterWebContentsDelegateAndroidJni },
  { "XWalkAutofillClient", RegisterXWalkAutofillClient },
  { "XWalkContentsClientBridge", RegisterXWalkContentsClientBridge },
  { "XWalkContent", RegisterXWalkContent },
  { "XWalkPresentationHost", RegisterXWalkPresentationHost },
  { "XWalkDevToolsServer", RegisterXWalkDevToolsServer },
  { "XWalkExtensionAndroid", extensions::RegisterXWalkExtensionAndroid },
  { "XWalkHttpAuthHandler", RegisterXWalkHttpAuthHandler },
  { "XWalkNativeExtensionLoaderAndroid",
      extensions::RegisterXWalkNativeExtensionLoaderAndroid },
  { "XWalkPathHelper", RegisterXWalkPathHelper },
  { "XWalkSettings", RegisterXWalkSettings },
  { "XWalkViewDelegate", RegisterXWalkViewDelegate },
};

bool RegisterJni(JNIEnv* env) {
  return RegisterNativeMethods(env,
    kXWalkRegisteredMethods, arraysize(kXWalkRegisteredMethods));
}

}  // namespace xwalk
