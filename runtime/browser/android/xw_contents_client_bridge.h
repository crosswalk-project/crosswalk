// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_XW_CONTENTS_CLIENT_BRIDGE_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_XW_CONTENTS_CLIENT_BRIDGE_H_

#include <jni.h>

#include "base/android/jni_helper.h"
#include "base/android/scoped_java_ref.h"
#include "base/callback.h"
#include "base/id_map.h"
#include "content/public/browser/javascript_dialog_manager.h"
#include "xwalk/runtime/browser/android/xw_contents_client_bridge_base.h"

namespace net {
class X509Certificate;
}

namespace xwalk {

// A class that handles the Java<->Native communication for the
// WebViewContentsClient. XwContentsClientBridge is created and owned by
// native WebViewContents class and it only has a weak reference to the
// its Java peer. Since the Java XwContentsClientBridge can have
// indirect refs from the Application (via callbacks) and so can outlive
// webview, this class notifies it before being destroyed and to nullify
// any references.
class XwContentsClientBridge : public XwContentsClientBridgeBase {
 public:
  XwContentsClientBridge(JNIEnv* env, jobject obj);
  virtual ~XwContentsClientBridge();

  // XwContentsClientBridgeBase implementation
  virtual void AllowCertificateError(int cert_error,
                                     net::X509Certificate* cert,
                                     const GURL& request_url,
                                     const base::Callback<void(bool)>& callback,
                                     bool* cancel_request) OVERRIDE;

  virtual void RunJavaScriptDialog(
      content::JavaScriptMessageType message_type,
      const GURL& origin_url,
      const string16& message_text,
      const string16& default_prompt_text,
      const content::JavaScriptDialogManager::DialogClosedCallback& callback)
      OVERRIDE;
  virtual void RunBeforeUnloadDialog(
      const GURL& origin_url,
      const string16& message_text,
      const content::JavaScriptDialogManager::DialogClosedCallback& callback)
      OVERRIDE;

  // Methods called from Java.
  void ProceedSslError(JNIEnv* env, jobject obj, jboolean proceed, jint id);
  void ConfirmJsResult(JNIEnv*, jobject, int id, jstring prompt);
  void CancelJsResult(JNIEnv*, jobject, int id);

 private:
  JavaObjectWeakGlobalRef java_ref_;

  typedef const base::Callback<void(bool)> CertErrorCallback;
  IDMap<CertErrorCallback, IDMapOwnPointer> pending_cert_error_callbacks_;
  IDMap<content::JavaScriptDialogManager::DialogClosedCallback, IDMapOwnPointer>
      pending_js_dialog_callbacks_;
};

bool RegisterXwContentsClientBridge(JNIEnv* env);

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_XW_CONTENTS_CLIENT_BRIDGE_H_
