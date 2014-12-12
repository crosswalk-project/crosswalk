// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_CONTENT_BROWSER_CLIENT_H_
#define XWALK_RUNTIME_BROWSER_XWALK_CONTENT_BROWSER_CLIENT_H_

#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/common/main_function_params.h"
#include "xwalk/runtime/browser/runtime_geolocation_permission_context.h"
#include "xwalk/runtime/browser/runtime_resource_dispatcher_host_delegate.h"

namespace content {
class BrowserContext;
class ResourceContext;
class QuotaPermissionContext;
class SpeechRecognitionManagerDelegate;
class WebContents;
class WebContentsViewDelegate;
}

namespace net {
class URLRequestContextGetter;
}

namespace xwalk {

class XWalkBrowserContext;
class XWalkBrowserMainParts;
class XWalkRunner;

class XWalkContentBrowserClient : public content::ContentBrowserClient {
 public:
  static XWalkContentBrowserClient* Get();

  explicit XWalkContentBrowserClient(XWalkRunner* xwalk_runner);
  virtual ~XWalkContentBrowserClient();

  // ContentBrowserClient overrides.
  content::BrowserMainParts* CreateBrowserMainParts(
      const content::MainFunctionParams& parameters) override;
  net::URLRequestContextGetter* CreateRequestContext(
      content::BrowserContext* browser_context,
      content::ProtocolHandlerMap* protocol_handlers,
      content::URLRequestInterceptorScopedVector request_interceptors) override;
  net::URLRequestContextGetter* CreateRequestContextForStoragePartition(
      content::BrowserContext* browser_context,
      const base::FilePath& partition_path,
      bool in_memory,
      content::ProtocolHandlerMap* protocol_handlers,
      content::URLRequestInterceptorScopedVector request_interceptors) override;
  void AppendExtraCommandLineSwitches(CommandLine* command_line,
                                      int child_process_id) override;
  content::QuotaPermissionContext*
      CreateQuotaPermissionContext() override;
  content::AccessTokenStore* CreateAccessTokenStore() override;
  content::WebContentsViewDelegate* GetWebContentsViewDelegate(
      content::WebContents* web_contents) override;
  void RenderProcessWillLaunch(
      content::RenderProcessHost* host) override;
  content::MediaObserver* GetMediaObserver() override;

  bool AllowGetCookie(const GURL& url,
                      const GURL& first_party,
                      const net::CookieList& cookie_list,
                      content::ResourceContext* context,
                      int render_process_id,
                      int render_frame_id) override;
  bool AllowSetCookie(const GURL& url,
                      const GURL& first_party,
                      const std::string& cookie_line,
                      content::ResourceContext* context,
                      int render_process_id,
                      int render_frame_id,
                      net::CookieOptions* options) override;

  void AllowCertificateError(
      int render_process_id,
      int render_frame_id,
      int cert_error,
      const net::SSLInfo& ssl_info,
      const GURL& request_url,
      content::ResourceType resource_type,
      bool overridable,
      bool strict_enforcement,
      bool expired_previous_decision,
      const base::Callback<void(bool)>& callback, // NOLINT
      content::CertificateRequestResultType* result) override;

  content::SpeechRecognitionManagerDelegate*
      GetSpeechRecognitionManagerDelegate() override;

  blink::WebNotificationPermission
  CheckDesktopNotificationPermission(
      const GURL& source_url,
      content::ResourceContext* context,
      int render_process_id) override;
  void ShowDesktopNotification(
      const content::ShowDesktopNotificationHostMsgParams& params,
      content::BrowserContext* browser_context,
      int render_process_id,
      scoped_ptr<content::DesktopNotificationDelegate> delegate,
      base::Closure* cancel_callback) override;
  void RequestPermission(
      content::PermissionType permission,
      content::WebContents* web_contents,
      int bridge_id,
      const GURL& requesting_frame,
      bool user_gesture,
      const base::Callback<void(bool)>& result_callback) override;
  void CancelPermissionRequest(
      content::PermissionType permission,
      content::WebContents* web_contents,
      int bridge_id,
      const GURL& requesting_frame) override;
#if !defined(OS_ANDROID)
  bool CanCreateWindow(const GURL& opener_url,
                       const GURL& opener_top_level_frame_url,
                       const GURL& source_origin,
                       WindowContainerType container_type,
                       const GURL& target_url,
                       const content::Referrer& referrer,
                       WindowOpenDisposition disposition,
                       const blink::WebWindowFeatures& features,
                       bool user_gesture,
                       bool opener_suppressed,
                       content::ResourceContext* context,
                       int render_process_id,
                       int opener_id,
                       bool* no_javascript_access) override;
#endif

  void DidCreatePpapiPlugin(
      content::BrowserPpapiHost* browser_host) override;
  content::BrowserPpapiHost* GetExternalBrowserPpapiHost(
      int plugin_process_id) override;

#if defined(OS_ANDROID) || defined(OS_TIZEN)  || defined(OS_LINUX)
  void ResourceDispatcherHostCreated() override;
#endif

  content::LocationProvider* OverrideSystemLocationProvider() override;

  void GetStoragePartitionConfigForSite(
      content::BrowserContext* browser_context,
      const GURL& site,
      bool can_be_default,
      std::string* partition_domain,
      std::string* partition_name,
      bool* in_memory) override;

  content::DevToolsManagerDelegate*
      GetDevToolsManagerDelegate() override;

  XWalkBrowserMainParts* main_parts() { return main_parts_; }

#if defined(OS_ANDROID)
  RuntimeResourceDispatcherHostDelegate* resource_dispatcher_host_delegate() {
    return resource_dispatcher_host_delegate_.get();
  }
#endif

 private:
  XWalkRunner* xwalk_runner_;
  net::URLRequestContextGetter* url_request_context_getter_;
  scoped_refptr<RuntimeGeolocationPermissionContext>
    geolocation_permission_context_;
  XWalkBrowserMainParts* main_parts_;
  XWalkBrowserContext* browser_context_;

  scoped_ptr<RuntimeResourceDispatcherHostDelegate>
      resource_dispatcher_host_delegate_;

  DISALLOW_COPY_AND_ASSIGN(XWalkContentBrowserClient);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_CONTENT_BROWSER_CLIENT_H_
