// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_CONTENT_BROWSER_CLIENT_H_
#define XWALK_RUNTIME_BROWSER_XWALK_CONTENT_BROWSER_CLIENT_H_

#include <map>
#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "base/files/scoped_file.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/common/main_function_params.h"
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
  ~XWalkContentBrowserClient() override;

  // ContentBrowserClient overrides.
  content::BrowserMainParts* CreateBrowserMainParts(
      const content::MainFunctionParams& parameters) override;
  void AppendExtraCommandLineSwitches(base::CommandLine* command_line,
                                      int child_process_id) override;
  content::QuotaPermissionContext*
      CreateQuotaPermissionContext() override;
  content::GeolocationDelegate* CreateGeolocationDelegate() override;
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
                      const net::CookieOptions& options) override;

  void AllowCertificateError(
      content::WebContents* web_contents,
      int cert_error,
      const net::SSLInfo& ssl_info,
      const GURL& request_url,
      content::ResourceType resource_type,
      bool overridable,
      bool strict_enforcement,
      bool expired_previous_decision,
      const base::Callback<void(bool)>& callback, // NOLINT
      content::CertificateRequestResultType* result) override;

  void SelectClientCertificate(
      content::WebContents* web_contents,
      net::SSLCertRequestInfo* cert_request_info,
      std::unique_ptr<content::ClientCertificateDelegate> delegate) override;

  content::SpeechRecognitionManagerDelegate*
      CreateSpeechRecognitionManagerDelegate() override;

  content::PlatformNotificationService* GetPlatformNotificationService()
      override;

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
                       int opener_render_view_id,
                       int opener_render_frame_id,
                       bool* no_javascript_access) override;
#endif

  void DidCreatePpapiPlugin(
      content::BrowserPpapiHost* browser_host) override;
  content::BrowserPpapiHost* GetExternalBrowserPpapiHost(
      int plugin_process_id) override;

#if defined(OS_ANDROID) || defined(OS_LINUX)
  void ResourceDispatcherHostCreated() override;
#endif

  void GetStoragePartitionConfigForSite(
      content::BrowserContext* browser_context,
      const GURL& site,
      bool can_be_default,
      std::string* partition_domain,
      std::string* partition_name,
      bool* in_memory) override;

  void GetAdditionalAllowedSchemesForFileSystem(
      std::vector<std::string>* additional_schemes) override;

#if defined(OS_ANDROID)
  virtual void GetAdditionalMappedFilesForChildProcess(
      const base::CommandLine& command_line,
      int child_process_id,
      content::FileDescriptorInfo* mappings,
      std::map<int, base::MemoryMappedFile::Region>* regions) override {}
#elif defined(OS_POSIX) && !defined(OS_MACOSX)
  void GetAdditionalMappedFilesForChildProcess(
      const base::CommandLine& command_line,
      int child_process_id,
      content::FileDescriptorInfo* mappings) override {}
#endif

  XWalkBrowserMainParts* main_parts() { return main_parts_; }

  content::PresentationServiceDelegate* GetPresentationServiceDelegate(
      content::WebContents* web_contents) override;

#if defined(OS_ANDROID)
  RuntimeResourceDispatcherHostDelegate* resource_dispatcher_host_delegate() {
    return resource_dispatcher_host_delegate_.get();
  }
#endif

  std::string GetApplicationLocale() override;

#if defined(OS_ANDROID)
  ScopedVector<content::NavigationThrottle> CreateThrottlesForNavigation(
      content::NavigationHandle* navigation_handle) override;
#endif

 private:
  XWalkRunner* xwalk_runner_;
  std::unique_ptr<content::ClientCertificateDelegate> delegate_;
#if defined(OS_POSIX) && !defined(OS_MACOSX)
  base::ScopedFD v8_natives_fd_;
  base::ScopedFD v8_snapshot_fd_;
#endif

  XWalkBrowserMainParts* main_parts_;

  std::unique_ptr<RuntimeResourceDispatcherHostDelegate>
      resource_dispatcher_host_delegate_;

  DISALLOW_COPY_AND_ASSIGN(XWalkContentBrowserClient);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_CONTENT_BROWSER_CLIENT_H_
