// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_content_browser_client.h"

#include <string>
#include <vector>

#include "base/command_line.h"
#include "base/path_service.h"
#include "base/files/file.h"
#include "content/public/browser/browser_child_process_host.h"
#include "content/public/browser/browser_main_parts.h"
#include "content/public/browser/browser_ppapi_host.h"
#include "content/public/browser/child_process_data.h"
#include "content/public/browser/client_certificate_delegate.h"
#include "content/public/browser/geolocation_delegate.h"
#include "content/public/browser/presentation_service_delegate.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/resource_context.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_descriptors.h"
#include "content/public/common/main_function_params.h"
#include "gin/v8_initializer.h"
#include "gin/public/isolate_holder.h"
#include "net/ssl/ssl_info.h"
#include "net/url_request/url_request_context_getter.h"
#include "ppapi/host/ppapi_host.h"
#include "xwalk/extensions/common/xwalk_extension_switches.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/runtime/browser/geolocation/xwalk_access_token_store.h"
#include "xwalk/runtime/browser/media/media_capture_devices_dispatcher.h"
#include "xwalk/runtime/browser/renderer_host/pepper/xwalk_browser_pepper_host_factory.h"
#include "xwalk/runtime/browser/runtime_platform_util.h"
#include "xwalk/runtime/browser/runtime_quota_permission_context.h"
#include "xwalk/runtime/browser/ssl_error_page.h"
#include "xwalk/runtime/browser/speech/speech_recognition_manager_delegate.h"
#include "xwalk/runtime/browser/xwalk_browser_context.h"
#include "xwalk/runtime/browser/xwalk_browser_main_parts.h"
#include "xwalk/runtime/browser/xwalk_platform_notification_service.h"
#include "xwalk/runtime/browser/xwalk_render_message_filter.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/runtime/common/xwalk_paths.h"
#include "xwalk/runtime/common/xwalk_switches.h"

#if !defined(DISABLE_NACL)
#include "components/nacl/browser/nacl_browser.h"
#include "components/nacl/browser/nacl_host_message_filter.h"
#include "components/nacl/browser/nacl_process_host.h"
#include "components/nacl/common/nacl_process_type.h"
#endif

#if defined(OS_ANDROID)
#include "base/android/locale_utils.h"
#include "base/android/path_utils.h"
#include "base/android/jni_android.h"
#include "base/android/jni_array.h"
#include "base/android/jni_string.h"
#include "base/base_paths_android.h"
#include "components/cdm/browser/cdm_message_filter_android.h"
#include "components/navigation_interception/intercept_navigation_delegate.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/navigation_throttle.h"
#include "xwalk/runtime/browser/android/xwalk_cookie_access_policy.h"
#include "xwalk/runtime/browser/android/xwalk_contents_client_bridge.h"
#include "xwalk/runtime/browser/android/xwalk_web_contents_view_delegate.h"
#include "xwalk/runtime/browser/xwalk_browser_main_parts_android.h"
#include "xwalk/runtime/common/android/xwalk_globals_android.h"
#else
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/application.h"
#endif

#if defined(OS_MACOSX)
#include "xwalk/runtime/browser/xwalk_browser_main_parts_mac.h"
#endif

#if defined(OS_ANDROID)
#include "xwalk/runtime/browser/xwalk_presentation_service_delegate_android.h"
#elif defined(OS_WIN)
#include "xwalk/runtime/browser/xwalk_presentation_service_delegate_win.h"
#endif

namespace xwalk {

namespace {

// The application-wide singleton of ContentBrowserClient impl.
XWalkContentBrowserClient* g_browser_client = nullptr;

// A provider of services for Geolocation.
class XWalkGeolocationDelegate : public content::GeolocationDelegate {
 public:
  explicit XWalkGeolocationDelegate(net::URLRequestContextGetter* request_context)
      : request_context_(request_context) {}

  content::AccessTokenStore* CreateAccessTokenStore() final {
    return new XWalkAccessTokenStore(request_context_);
  }

 private:
  net::URLRequestContextGetter* request_context_;

  DISALLOW_COPY_AND_ASSIGN(XWalkGeolocationDelegate);
};

}  // namespace

// static
XWalkContentBrowserClient* XWalkContentBrowserClient::Get() {
  return g_browser_client;
}

XWalkContentBrowserClient::XWalkContentBrowserClient(XWalkRunner* xwalk_runner)
    : xwalk_runner_(xwalk_runner),
#if defined(OS_POSIX) && !defined(OS_MACOSX)
      v8_natives_fd_(-1),
      v8_snapshot_fd_(-1),
#endif  // OS_POSIX && !OS_MACOSX
      main_parts_(nullptr) {
  DCHECK(!g_browser_client);
  g_browser_client = this;
}

XWalkContentBrowserClient::~XWalkContentBrowserClient() {
  DCHECK(g_browser_client);
  g_browser_client = nullptr;
}

content::BrowserMainParts* XWalkContentBrowserClient::CreateBrowserMainParts(
    const content::MainFunctionParams& parameters) {
#if defined(OS_MACOSX)
  main_parts_ = new XWalkBrowserMainPartsMac(parameters);
#elif defined(OS_ANDROID)
  main_parts_ = new XWalkBrowserMainPartsAndroid(parameters);
#else
  main_parts_ = new XWalkBrowserMainParts(parameters);
#endif

  return main_parts_;
}

// This allow us to append extra command line switches to the child
// process we launch.
void XWalkContentBrowserClient::AppendExtraCommandLineSwitches(
    base::CommandLine* command_line, int child_process_id) {
  const base::CommandLine& browser_process_cmd_line =
      *base::CommandLine::ForCurrentProcess();
  const char* extra_switches[] = {
    switches::kXWalkDisableExtensionProcess,
#if defined(ENABLE_PLUGINS)
    switches::kPpapiFlashPath,
    switches::kPpapiFlashVersion
#endif
  };

  command_line->CopySwitchesFrom(
      browser_process_cmd_line, extra_switches, arraysize(extra_switches));
}

content::QuotaPermissionContext*
XWalkContentBrowserClient::CreateQuotaPermissionContext() {
  return new RuntimeQuotaPermissionContext();
}

content::GeolocationDelegate*
XWalkContentBrowserClient::CreateGeolocationDelegate() {
  return new XWalkGeolocationDelegate(
      xwalk_runner_->browser_context()->url_request_getter());
}

content::WebContentsViewDelegate*
XWalkContentBrowserClient::GetWebContentsViewDelegate(
    content::WebContents* web_contents) {
#if defined(OS_ANDROID)
  return new XWalkWebContentsViewDelegate(web_contents);
#else
  return nullptr;
#endif
}

void XWalkContentBrowserClient::RenderProcessWillLaunch(
    content::RenderProcessHost* host) {
#if !defined(DISABLE_NACL)
  int id = host->GetID();
  net::URLRequestContextGetter* context =
      host->GetStoragePartition()->GetURLRequestContext();

  host->AddFilter(new nacl::NaClHostMessageFilter(
      id,
      // TODO(Halton): IsOffTheRecord?
      false,
      host->GetBrowserContext()->GetPath(),
      context));
#endif
  xwalk_runner_->OnRenderProcessWillLaunch(host);
  host->AddFilter(new XWalkRenderMessageFilter);
#if defined(OS_ANDROID)
  host->AddFilter(new cdm::CdmMessageFilterAndroid());
  host->AddFilter(new XWalkRenderMessageFilter(host->GetID()));
#endif
}

content::MediaObserver* XWalkContentBrowserClient::GetMediaObserver() {
  return XWalkMediaCaptureDevicesDispatcher::GetInstance();
}

bool XWalkContentBrowserClient::AllowGetCookie(
    const GURL& url,
    const GURL& first_party,
    const net::CookieList& cookie_list,
    content::ResourceContext* context,
    int render_process_id,
    int render_frame_id) {
#if defined(OS_ANDROID)
  return XWalkCookieAccessPolicy::GetInstance()->AllowGetCookie(
      url,
      first_party,
      cookie_list,
      context,
      render_process_id,
      render_frame_id);
#else
  return true;
#endif
}

bool XWalkContentBrowserClient::AllowSetCookie(
    const GURL& url,
    const GURL& first_party,
    const std::string& cookie_line,
    content::ResourceContext* context,
    int render_process_id,
    int render_frame_id,
    const net::CookieOptions& options) {
#if defined(OS_ANDROID)
  return XWalkCookieAccessPolicy::GetInstance()->AllowSetCookie(
      url,
      first_party,
      cookie_line,
      context,
      render_process_id,
      render_frame_id,
      options);
#else
  return true;
#endif
}

// Selects a SSL client certificate and returns it to the |delegate|. If no
// certificate was selected NULL is returned to the |delegate|.
void XWalkContentBrowserClient::SelectClientCertificate(
    content::WebContents* web_contents,
    net::SSLCertRequestInfo* cert_request_info,
    std::unique_ptr<content::ClientCertificateDelegate> delegate) {
#if defined(OS_ANDROID)
  XWalkContentsClientBridgeBase* client =
      XWalkContentsClientBridgeBase::FromWebContents(web_contents);
  if (client) {
    client->SelectClientCertificate(cert_request_info, std::move(delegate));
  } else {
    delegate->ContinueWithCertificate(nullptr);
  }
#endif
}

void XWalkContentBrowserClient::AllowCertificateError(
    content::WebContents* web_contents,
    int cert_error,
    const net::SSLInfo& ssl_info,
    const GURL& request_url,
    content::ResourceType resource_type,
    bool overridable,
    bool strict_enforcement,
    bool expired_previous_decision,
    const base::Callback<void(bool)>& callback, // NOLINT
    content::CertificateRequestResultType* result) {
  // Currently only Android handles it.
  // TODO(yongsheng): applies it for other platforms?
#if defined(OS_ANDROID)
  XWalkContentsClientBridgeBase* client =
      XWalkContentsClientBridgeBase::FromWebContents(web_contents);
  bool cancel_request = true;
  if (client)
    client->AllowCertificateError(cert_error,
                                  ssl_info.cert.get(),
                                  request_url,
                                  callback,
                                  &cancel_request);
  if (cancel_request)
    *result = content::CERTIFICATE_REQUEST_RESULT_TYPE_DENY;
#else
  DCHECK(web_contents);
  // The interstitial page shown is responsible for destroying
  // this instance of SSLErrorPage
  (new SSLErrorPage(web_contents, cert_error,
                    ssl_info, request_url, callback))->Show();
#endif
}

content::PlatformNotificationService*
XWalkContentBrowserClient::GetPlatformNotificationService() {
  return XWalkPlatformNotificationService::GetInstance();
}

void XWalkContentBrowserClient::DidCreatePpapiPlugin(
    content::BrowserPpapiHost* browser_host) {
#if defined(ENABLE_PLUGINS)
  browser_host->GetPpapiHost()->AddHostFactoryFilter(
      std::unique_ptr<ppapi::host::HostFactory>(
          new XWalkBrowserPepperHostFactory(browser_host)));
#endif
}

content::BrowserPpapiHost*
    XWalkContentBrowserClient::GetExternalBrowserPpapiHost(
        int plugin_process_id) {
#if !defined(DISABLE_NACL)
  content::BrowserChildProcessHostIterator iter(PROCESS_TYPE_NACL_LOADER);
  while (!iter.Done()) {
    nacl::NaClProcessHost* host = static_cast<nacl::NaClProcessHost*>(
        iter.GetDelegate());
    if (host->process() &&
        host->process()->GetData().id == plugin_process_id) {
      // Found the plugin.
      return host->browser_ppapi_host();
    }
    ++iter;
  }
#endif
  return nullptr;
}

#if defined(OS_ANDROID) || defined(OS_LINUX)
void XWalkContentBrowserClient::ResourceDispatcherHostCreated() {
  resource_dispatcher_host_delegate_ =
      (RuntimeResourceDispatcherHostDelegate::Create());
  content::ResourceDispatcherHost::Get()->SetDelegate(
      resource_dispatcher_host_delegate_.get());
}
#endif

content::SpeechRecognitionManagerDelegate*
    XWalkContentBrowserClient::CreateSpeechRecognitionManagerDelegate() {
  return new xwalk::XWalkSpeechRecognitionManagerDelegate();
}

#if !defined(OS_ANDROID)
bool XWalkContentBrowserClient::CanCreateWindow(const GURL& opener_url,
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
                             bool* no_javascript_access) {
  *no_javascript_access = false;
  application::Application* app = xwalk_runner_->app_system()->
      application_service()->GetApplicationByRenderHostID(render_process_id);
  if (!app)
    // If it's not a request from an application, always enable this action.
    return true;

  if (app->CanRequestURL(target_url)) {
    LOG(INFO) << "[ALLOW] CreateWindow: " << target_url.spec();
    return true;
  }

  LOG(INFO) << "[BlOCK] CreateWindow: " << target_url.spec();
  platform_util::OpenExternal(target_url);
  return false;
}
#endif

void XWalkContentBrowserClient::GetStoragePartitionConfigForSite(
    content::BrowserContext* browser_context,
    const GURL& site,
    bool can_be_default,
    std::string* partition_domain,
    std::string* partition_name,
    bool* in_memory) {
  *in_memory = false;
  partition_domain->clear();
  partition_name->clear();

#if !defined(OS_ANDROID)
  if (site.SchemeIs(application::kApplicationScheme))
    *partition_domain = site.host();
#endif
}

void XWalkContentBrowserClient::GetAdditionalAllowedSchemesForFileSystem(
    std::vector<std::string>* additional_schemes) {
#if !defined(OS_ANDROID)
  additional_schemes->push_back("app");
#endif
}

content::PresentationServiceDelegate* XWalkContentBrowserClient::
    GetPresentationServiceDelegate(content::WebContents* web_contents) {
#if defined(OS_WIN)
  return XWalkPresentationServiceDelegateWin::
      GetOrCreateForWebContents(web_contents);
#elif defined(OS_ANDROID)
  return XWalkPresentationServiceDelegateAndroid::
      GetOrCreateForWebContents(web_contents);
#else
  return nullptr;
#endif
}

std::string XWalkContentBrowserClient::GetApplicationLocale() {
#if defined(OS_ANDROID)
  return base::android::GetDefaultLocale();
#else
  return content::ContentBrowserClient::GetApplicationLocale();
#endif
}

#if defined(OS_ANDROID)
ScopedVector<content::NavigationThrottle>
XWalkContentBrowserClient::CreateThrottlesForNavigation(
    content::NavigationHandle* navigation_handle) {
  ScopedVector<content::NavigationThrottle> throttles;
  // We allow intercepting only navigations within main frames. This
  // is used to post onPageStarted. We handle shouldOverrideUrlLoading
  // via a sync IPC.
  if (navigation_handle->IsInMainFrame()) {
    throttles.push_back(
        navigation_interception::InterceptNavigationDelegate::CreateThrottleFor(
            navigation_handle));
  }
  return throttles;
}
#endif

}  // namespace xwalk
