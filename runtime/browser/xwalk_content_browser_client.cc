// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_content_browser_client.h"

#include <vector>

#include "base/command_line.h"
#include "base/path_service.h"
#include "base/platform_file.h"
#include "xwalk/extensions/browser/xwalk_extension_service.h"
#include "xwalk/extensions/common/xwalk_extension_switches.h"
#include "xwalk/runtime/browser/xwalk_browser_main_parts.h"
#include "xwalk/runtime/browser/geolocation/xwalk_access_token_store.h"
#include "xwalk/runtime/browser/media/media_capture_devices_dispatcher.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/browser/runtime_quota_permission_context.h"
#include "content/public/browser/browser_main_parts.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/main_function_params.h"
#include "net/url_request/url_request_context_getter.h"

#if defined(OS_ANDROID)
#include "base/android/path_utils.h"
#include "base/base_paths_android.h"
#include "xwalk/runtime/browser/runtime_resource_dispatcher_host_delegate.h"
#include "xwalk/runtime/browser/xwalk_browser_main_parts_android.h"
#include "xwalk/runtime/common/android/xwalk_globals_android.h"
#endif

#if defined(OS_MACOSX)
#include "xwalk/runtime/browser/xwalk_browser_main_parts_mac.h"
#endif

#if defined(OS_TIZEN_MOBILE)
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


XWalkContentBrowserClient::XWalkContentBrowserClient()
    : main_parts_(NULL) {
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
#elif defined(OS_TIZEN_MOBILE)
  main_parts_ = new XWalkBrowserMainPartsTizen(parameters);
#else
  main_parts_ = new XWalkBrowserMainParts(parameters);
#endif

  return main_parts_;
}

net::URLRequestContextGetter* XWalkContentBrowserClient::CreateRequestContext(
    content::BrowserContext* browser_context,
    content::ProtocolHandlerMap* protocol_handlers) {
  url_request_context_getter_ = static_cast<RuntimeContext*>(browser_context)->
      CreateRequestContext(protocol_handlers);
  return url_request_context_getter_;
}

net::URLRequestContextGetter*
XWalkContentBrowserClient::CreateRequestContextForStoragePartition(
    content::BrowserContext* browser_context,
    const base::FilePath& partition_path,
    bool in_memory,
    content::ProtocolHandlerMap* protocol_handlers) {
  return static_cast<RuntimeContext*>(browser_context)->
      CreateRequestContextForStoragePartition(
          partition_path, in_memory, protocol_handlers);
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
  return NULL;
}

void XWalkContentBrowserClient::RenderProcessHostCreated(
    content::RenderProcessHost* host) {
  xwalk::extensions::XWalkExtensionService* extension_service =
      main_parts_->extension_service();
  if (extension_service)
    extension_service->OnRenderProcessHostCreated(host);
}

content::MediaObserver* XWalkContentBrowserClient::GetMediaObserver() {
  return XWalkMediaCaptureDevicesDispatcher::GetInstance();
}

#if defined(OS_ANDROID)
void XWalkContentBrowserClient::GetAdditionalMappedFilesForChildProcess(
    const CommandLine& command_line,
    int child_process_id,
    std::vector<content::FileDescriptorInfo>* mappings) {
  int flags = base::PLATFORM_FILE_OPEN | base::PLATFORM_FILE_READ;
  base::FilePath pak_file;
  bool r = PathService::Get(base::DIR_ANDROID_APP_DATA, &pak_file);
  CHECK(r);
  pak_file = pak_file.Append(FILE_PATH_LITERAL("paks"));
  pak_file = pak_file.Append(FILE_PATH_LITERAL(kXWalkPakFilePath));

  base::PlatformFile f =
      base::CreatePlatformFile(pak_file, flags, NULL, NULL);
  if (f == base::kInvalidPlatformFileValue) {
    NOTREACHED() << "Failed to open file when creating renderer process: "
                 << "xwalk.pak";
  }
  mappings->push_back(
      content::FileDescriptorInfo(kXWalkPakDescriptor,
                                  base::FileDescriptor(f, true)));
}

void XWalkContentBrowserClient::ResourceDispatcherHostCreated() {
  RuntimeResourceDispatcherHostDelegate::ResourceDispatcherHostCreated();
}
#endif

void XWalkContentBrowserClient::RenderProcessHostGone(
    content::RenderProcessHost* host) {
  xwalk::extensions::XWalkExtensionService* extension_service =
      main_parts_->extension_service();
  if (extension_service)
    extension_service->OnRenderProcessDied(host);
}

}  // namespace xwalk
