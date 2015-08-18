// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_XWALK_CONTENTS_CLIENT_BRIDGE_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_XWALK_CONTENTS_CLIENT_BRIDGE_H_

#include <jni.h>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "base/android/jni_weak_ref.h"
#include "base/android/scoped_java_ref.h"
#include "base/callback.h"
#include "base/id_map.h"
#include "content/public/browser/javascript_dialog_manager.h"
#include "xwalk/runtime/browser/android/xwalk_contents_client_bridge_base.h"
#include "xwalk/runtime/browser/android/xwalk_icon_helper.h"

namespace gfx {
class Size;
}

namespace net {
class X509Certificate;
}

namespace content {
class WebContents;
}

class SkBitmap;

namespace xwalk {

// A class that handles the Java<->Native communication for the
// XWalkContentsClient. XWalkContentsClientBridge is created and owned by
// native XWalkViewContents class and it only has a weak reference to the
// its Java peer. Since the Java XWalkContentsClientBridge can have
// indirect refs from the Application (via callbacks) and so can outlive
// XWalkView, this class notifies it before being destroyed and to nullify
// any references.
class XWalkContentsClientBridge : public XWalkContentsClientBridgeBase ,
                                  public XWalkIconHelper::Listener {
 public:
  XWalkContentsClientBridge(JNIEnv* env, jobject obj,
                            content::WebContents* web_contents);
  ~XWalkContentsClientBridge() override;

  // XWalkContentsClientBridgeBase implementation
  void AllowCertificateError(int cert_error,
                             net::X509Certificate* cert,
                             const GURL& request_url,
                             const base::Callback<void(bool)>& callback, // NOLINT
                             bool* cancel_request) override;

  void RunJavaScriptDialog(
      content::JavaScriptMessageType message_type,
      const GURL& origin_url,
      const base::string16& message_text,
      const base::string16& default_prompt_text,
      const content::JavaScriptDialogManager::DialogClosedCallback& callback)
      override;
  void RunBeforeUnloadDialog(
      const GURL& origin_url,
      const base::string16& message_text,
      const content::JavaScriptDialogManager::DialogClosedCallback& callback)
      override;
  void ShowNotification(
      const content::PlatformNotificationData& notification_data,
      const SkBitmap& icon,
      scoped_ptr<content::DesktopNotificationDelegate> delegate,
      base::Closure* cancel_callback)
      override;
  void OnWebLayoutPageScaleFactorChanged(
      float page_scale_factor)
      override;

  bool OnReceivedHttpAuthRequest(const base::android::JavaRef<jobject>& handler,
                                 const std::string& host,
                                 const std::string& realm);

  // Methods called from Java.
  void ProceedSslError(JNIEnv* env, jobject obj, jboolean proceed, jint id);
  void ConfirmJsResult(JNIEnv*, jobject, int id, jstring prompt);
  void CancelJsResult(JNIEnv*, jobject, int id);
  void ExitFullscreen(JNIEnv*, jobject, jlong web_contents);
  void NotificationDisplayed(JNIEnv*, jobject, jint id);
  void NotificationClicked(JNIEnv*, jobject, jint id);
  void NotificationClosed(JNIEnv*, jobject, jint id, bool by_user);
  void OnFilesSelected(
      JNIEnv*, jobject, int process_id, int render_id,
      int mode, jstring filepath, jstring display_name);
  void OnFilesNotSelected(
      JNIEnv*, jobject, int process_id, int render_id, int mode);
  void DownloadIcon(JNIEnv* env, jobject obj, jstring url);

  // XWalkIconHelper::Listener Interface
  virtual void OnIconAvailable(const GURL& icon_url);
  virtual void OnReceivedIcon(const GURL& icon_url, const SkBitmap& bitmap);

  void ProvideClientCertificateResponse(JNIEnv* env, jobject object,
      jint request_id, jobjectArray encoded_chain_ref,
      jobject private_key_ref);

  virtual void SelectClientCertificate(
      net::SSLCertRequestInfo* cert_request_info,
      scoped_ptr<content::ClientCertificateDelegate> delegate);

  void HandleErrorInClientCertificateResponse(int id);
 private:
  JavaObjectWeakGlobalRef java_ref_;

  typedef const base::Callback<void(bool)> CertErrorCallback; // NOLINT
  IDMap<CertErrorCallback, IDMapOwnPointer> pending_cert_error_callbacks_;
  IDMap<content::JavaScriptDialogManager::DialogClosedCallback, IDMapOwnPointer>
      pending_js_dialog_callbacks_;
  // |pending_client_cert_request_delegates_| owns its pointers, but IDMap
  // doesn't provide Release, so ownership is managed manually.
  IDMap<content::ClientCertificateDelegate>
      pending_client_cert_request_delegates_;

  typedef std::pair<int, content::RenderFrameHost*>
    NotificationDownloadRequestInfos;

  IDMap<SelectCertificateCallback, IDMapOwnPointer>
      pending_client_cert_request_callbacks_;

  scoped_ptr<XWalkIconHelper> icon_helper_;
};

bool RegisterXWalkContentsClientBridge(JNIEnv* env);

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_XWALK_CONTENTS_CLIENT_BRIDGE_H_
