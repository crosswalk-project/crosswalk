// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_XWALK_CONTENTS_CLIENT_BRIDGE_BASE_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_XWALK_CONTENTS_CLIENT_BRIDGE_BASE_H_

#include "base/callback_forward.h"
#include "base/supports_user_data.h"
#include "content/public/browser/client_certificate_delegate.h"
#include "content/public/browser/javascript_dialog_manager.h"
#include "net/ssl/ssl_cert_request_info.h"

class GURL;
class SkBitmap;

namespace content {
class DesktopNotificationDelegate;
struct NotificationResources;
struct PlatformNotificationData;
class RenderFrameHost;
class WebContents;
}

namespace net {
class X509Certificate;
}

namespace xwalk {

// browser/ layer interface for XWalkContentsClientBridge, as DEPS prevents
// this layer from depending on native/ where the implementation lives. The
// implementor of the base class plumbs the request to the Java side and
// eventually to the XWalkClient. This layering hides the details of
// native/ from browser/ layer.
class XWalkContentsClientBridgeBase {
 public:
  typedef base::Callback<void(net::X509Certificate*)> SelectCertificateCallback;
  // Adds the handler to the UserData registry.
  static void Associate(content::WebContents* web_contents,
                        XWalkContentsClientBridgeBase* handler);
  static XWalkContentsClientBridgeBase* FromWebContents(
      content::WebContents* web_contents);
  static XWalkContentsClientBridgeBase* FromRenderViewID(int render_process_id,
                                            int render_view_id);
  static XWalkContentsClientBridgeBase* FromRenderFrameID(int render_process_id,
                                            int render_frame_id);
  static XWalkContentsClientBridgeBase* FromRenderFrameHost(
      content::RenderFrameHost* render_frame_host);

  virtual ~XWalkContentsClientBridgeBase();

  virtual void SelectClientCertificate(
      net::SSLCertRequestInfo* cert_request_info,
      std::unique_ptr<content::ClientCertificateDelegate> delegate) = 0;

  virtual void AllowCertificateError(int cert_error,
                                     net::X509Certificate* cert,
                                     const GURL& request_url,
                                     const base::Callback<void(bool)>& callback,
                                     bool* cancel_request) = 0;

  virtual void RunJavaScriptDialog(
      content::JavaScriptMessageType message_type,
      const GURL& origin_url,
      const base::string16& message_text,
      const base::string16& default_prompt_text,
      const content::JavaScriptDialogManager::DialogClosedCallback& callback)
      = 0;
  virtual void RunBeforeUnloadDialog(
      const GURL& origin_url,
      const content::JavaScriptDialogManager::DialogClosedCallback& callback)
      = 0;
  virtual void ShowNotification(
      const content::PlatformNotificationData& notification_data,
      const content::NotificationResources& notification_resources,
      std::unique_ptr<content::DesktopNotificationDelegate> delegate,
      base::Closure* cancel_callback)
      = 0;
  virtual void OnWebLayoutPageScaleFactorChanged(float page_scale_factor) = 0;
  virtual bool ShouldOverrideUrlLoading(const base::string16& url,
                                        bool has_user_gesture,
                                        bool is_redirect,
                                        bool is_main_frame) = 0;
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_XWALK_CONTENTS_CLIENT_BRIDGE_BASE_H_
