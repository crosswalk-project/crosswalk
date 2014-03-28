// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_content_browser_client.h"

#include <string>
#include <vector>

#include "base/command_line.h"
#include "base/path_service.h"
#include "base/platform_file.h"
#include "content/public/browser/browser_main_parts.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/resource_context.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/main_function_params.h"
#include "content/public/common/show_desktop_notification_params.h"
#include "net/ssl/ssl_info.h"
#include "net/url_request/url_request_context_getter.h"
#include "xwalk/extensions/common/xwalk_extension_switches.h"
#include "xwalk/runtime/browser/xwalk_browser_main_parts.h"
#include "xwalk/runtime/browser/geolocation/xwalk_access_token_store.h"
#include "xwalk/runtime/browser/media/media_capture_devices_dispatcher.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/runtime_quota_permission_context.h"
#include "xwalk/runtime/browser/speech/speech_recognition_manager_delegate.h"
#include "xwalk/runtime/browser/xwalk_runner.h"

#if defined(OS_ANDROID)
#include "base/android/path_utils.h"
#include "base/base_paths_android.h"
#include "xwalk/runtime/browser/android/xwalk_cookie_access_policy.h"
#include "xwalk/runtime/browser/android/xwalk_contents_client_bridge.h"
#include "xwalk/runtime/browser/android/xwalk_web_contents_view_delegate.h"
#include "xwalk/runtime/browser/runtime_resource_dispatcher_host_delegate_android.h"
#include "xwalk/runtime/browser/xwalk_browser_main_parts_android.h"
#include "xwalk/runtime/common/android/xwalk_globals_android.h"
#endif

#if defined(OS_MACOSX)
#include "xwalk/runtime/browser/xwalk_browser_main_parts_mac.h"
#endif

#if defined(OS_TIZEN)
#include "xwalk/runtime/browser/xwalk_browser_main_parts_tizen.h"
#endif

namespace xwalk {

namespace {

// The application-wide singleton of ContentBrowserClient impl.
XWalkContentBrowserClient* g_browser_client = NULL;

}  // namespace

// static
XWalkContentBrowserClient* XWalkContentBrowserClient::Get() {
  return g_browser_client;
}


XWalkContentBrowserClient::XWalkContentBrowserClient(XWalkRunner* xwalk_runner)
    : xwalk_runner_(xwalk_runner),
      url_request_context_getter_(NULL),
      main_parts_(NULL) {
  DCHECK(!g_browser_client);
  g_browser_client = this;
}

XWalkContentBrowserClient::~XWalkContentBrowserClient() {
  DCHECK(g_browser_client);
  g_browser_client = NULL;
}

content::BrowserMainParts* XWalkContentBrowserClient::CreateBrowserMainParts(
    const content::MainFunctionParams& parameters) {
#if defined(OS_MACOSX)
  main_parts_ = new XWalkBrowserMainPartsMac(parameters);
#elif defined(OS_ANDROID)
  main_parts_ = new XWalkBrowserMainPartsAndroid(parameters);
#elif defined(OS_TIZEN)
  main_parts_ = new XWalkBrowserMainPartsTizen(parameters);
#else
  main_parts_ = new XWalkBrowserMainParts(parameters);
#endif

  return main_parts_;
}

net::URLRequestContextGetter* XWalkContentBrowserClient::CreateRequestContext(
    content::BrowserContext* browser_context,
    content::ProtocolHandlerMap* protocol_handlers,
    content::ProtocolHandlerScopedVector protocol_interceptors) {
  url_request_context_getter_ = static_cast<RuntimeContext*>(browser_context)->
      CreateRequestContext(protocol_handlers, protocol_interceptors.Pass());
  return url_request_context_getter_;
}

net::URLRequestContextGetter*
XWalkContentBrowserClient::CreateRequestContextForStoragePartition(
    content::BrowserContext* browser_context,
    const base::FilePath& partition_path,
    bool in_memory,
    content::ProtocolHandlerMap* protocol_handlers,
    content::ProtocolHandlerScopedVector protocol_interceptors) {
  return static_cast<RuntimeContext*>(browser_context)->
      CreateRequestContextForStoragePartition(
          partition_path, in_memory, protocol_handlers, protocol_interceptors.Pass());
}

// This allow us to append extra command line switches to the child
// process we launch.
void XWalkContentBrowserClient::AppendExtraCommandLineSwitches(
    CommandLine* command_line, int child_process_id) {
  CommandLine* browser_process_cmd_line = CommandLine::ForCurrentProcess();
  const int extra_switches_count = 1;
  const char* extra_switches[extra_switches_count] = {
    switches::kXWalkDisableExtensionProcess
  };

  for (int i = 0; i < extra_switches_count; i++) {
    if (browser_process_cmd_line->HasSwitch(extra_switches[i]))
      command_line->AppendSwitch(extra_switches[i]);
  }
}

content::QuotaPermissionContext*
XWalkContentBrowserClient::CreateQuotaPermissionContext() {
  return new RuntimeQuotaPermissionContext();
}

content::AccessTokenStore* XWalkContentBrowserClient::CreateAccessTokenStore() {
  return new XWalkAccessTokenStore(url_request_context_getter_);
}

content::WebContentsViewDelegate*
XWalkContentBrowserClient::GetWebContentsViewDelegate(
    content::WebContents* web_contents) {
#if defined(OS_ANDROID)
  return new XWalkWebContentsViewDelegate(web_contents);
#else
  return NULL;
#endif
}

void XWalkContentBrowserClient::RenderProcessWillLaunch(
    content::RenderProcessHost* host) {
  xwalk_runner_->OnRenderProcessWillLaunch(host);
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
    net::CookieOptions* options) {
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

void XWalkContentBrowserClient::AllowCertificateError(
    int render_process_id,
    int render_frame_id,
    int cert_error,
    const net::SSLInfo& ssl_info,
    const GURL& request_url,
    ResourceType::Type resource_type,
    bool overridable,
    bool strict_enforcement,
    const base::Callback<void(bool)>& callback,
    content::CertificateRequestResultType* result) {
  // Currently only Android handles it.
  // TODO(yongsheng): applies it for other platforms?
#if defined(OS_ANDROID)
  XWalkContentsClientBridgeBase* client =
      XWalkContentsClientBridgeBase::FromRenderFrameID(render_process_id,
          render_frame_id);
  bool cancel_request = true;
  if (client)
    client->AllowCertificateError(cert_error,
                                  ssl_info.cert.get(),
                                  request_url,
                                  callback,
                                  &cancel_request);
  if (cancel_request)
    *result = content::CERTIFICATE_REQUEST_RESULT_TYPE_DENY;
#endif
}

void XWalkContentBrowserClient::RequestDesktopNotificationPermission(
    const GURL& source_origin,
    int callback_context,
    int render_process_id,
    int render_view_id) {
}

blink::WebNotificationPresenter::Permission
XWalkContentBrowserClient::CheckDesktopNotificationPermission(
    const GURL& source_url,
    content::ResourceContext* context,
    int render_process_id) {
#if defined(OS_ANDROID)
  return blink::WebNotificationPresenter::PermissionAllowed;
#else
  return blink::WebNotificationPresenter::PermissionNotAllowed;
#endif
}

void XWalkContentBrowserClient::ShowDesktopNotification(
    const content::ShowDesktopNotificationHostMsgParams& params,
    int render_process_id,
    int render_view_id,
    bool worker) {
#if defined(OS_ANDROID)
  XWalkContentsClientBridgeBase* bridge =
      XWalkContentsClientBridgeBase::FromRenderViewID(render_process_id,
          render_view_id);
  bridge->ShowNotification(params, worker, render_process_id, render_view_id);
#endif
}

void XWalkContentBrowserClient::CancelDesktopNotification(
    int render_process_id,
    int render_view_id,
    int notification_id) {
#if defined(OS_ANDROID)
  XWalkContentsClientBridgeBase* bridge =
      XWalkContentsClientBridgeBase::FromRenderViewID(render_process_id,
          render_view_id);
  bridge->CancelNotification(
      notification_id, render_process_id, render_view_id);
#endif
}

#if defined(OS_ANDROID)
void XWalkContentBrowserClient::ResourceDispatcherHostCreated() {
  RuntimeResourceDispatcherHostDelegateAndroid::
  ResourceDispatcherHostCreated();
}
#endif

content::SpeechRecognitionManagerDelegate*
    XWalkContentBrowserClient::GetSpeechRecognitionManagerDelegate() {
  return new xwalk::XWalkSpeechRecognitionManagerDelegate();
}

}  // namespace xwalk
