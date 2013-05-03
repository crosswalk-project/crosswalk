// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/shell_content_browser_client.h"

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/path_service.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/common/content_switches.h"
#include "content/shell/geolocation/shell_access_token_store.h"
#include "content/shell/shell.h"
#include "content/shell/shell_browser_context.h"
#include "content/shell/shell_browser_main_parts.h"
#include "content/shell/shell_devtools_delegate.h"
#include "content/shell/shell_message_filter.h"
#include "content/shell/shell_messages.h"
#include "content/shell/shell_quota_permission_context.h"
#include "content/shell/shell_resource_dispatcher_host_delegate.h"
#include "content/shell/shell_switches.h"
#include "content/shell/shell_web_contents_view_delegate_creator.h"
#include "content/shell/webkit_test_controller.h"
#include "googleurl/src/gurl.h"
#include "webkit/glue/webpreferences.h"

#if defined(OS_ANDROID)
#include "base/android/path_utils.h"
#include "base/path_service.h"
#include "base/platform_file.h"
#include "content/shell/android/shell_descriptors.h"
#endif

namespace content {

namespace {

ShellContentBrowserClient* g_browser_client;

base::FilePath GetWebKitRootDirFilePath() {
  base::FilePath base_path;
  PathService::Get(base::DIR_SOURCE_ROOT, &base_path);
  if (file_util::PathExists(
          base_path.Append(FILE_PATH_LITERAL("third_party/WebKit")))) {
    // We're in a WebKit-in-chrome checkout.
    return base_path.Append(FILE_PATH_LITERAL("third_party/WebKit"));
  } else if (file_util::PathExists(
          base_path.Append(FILE_PATH_LITERAL("chromium")))) {
    // We're in a WebKit-only checkout on Windows.
    return base_path.Append(FILE_PATH_LITERAL("../.."));
  } else if (file_util::PathExists(
          base_path.Append(FILE_PATH_LITERAL("webkit/support")))) {
    // We're in a WebKit-only/xcodebuild checkout on Mac
    return base_path.Append(FILE_PATH_LITERAL("../../.."));
  }
  // We're in a WebKit-only, make-build, so the DIR_SOURCE_ROOT is already the
  // WebKit root. That, or we have no idea where we are.
  return base_path;
}

base::FilePath GetChromiumRootDirFilePath() {
  base::FilePath webkit_path = GetWebKitRootDirFilePath();
  if (file_util::PathExists(webkit_path.Append(
          FILE_PATH_LITERAL("Source/WebKit/chromium/webkit/support")))) {
    // We're in a WebKit-only checkout.
    return webkit_path.Append(FILE_PATH_LITERAL("Source/WebKit/chromium"));
  } else {
    // We're in a Chromium checkout, and WebKit is in third_party/WebKit.
    return webkit_path.Append(FILE_PATH_LITERAL("../.."));
  }
}

}  // namespace

ShellContentBrowserClient* ShellContentBrowserClient::Get() {
  return g_browser_client;
}

ShellContentBrowserClient::ShellContentBrowserClient()
    : hyphen_dictionary_file_(base::kInvalidPlatformFileValue),
      shell_browser_main_parts_(NULL) {
  DCHECK(!g_browser_client);
  g_browser_client = this;
  if (!CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree))
    return;
  webkit_source_dir_ = GetWebKitRootDirFilePath();
  base::FilePath dictionary_file_path = GetChromiumRootDirFilePath().Append(
      FILE_PATH_LITERAL("third_party/hyphen/hyph_en_US.dic"));
  file_util::AbsolutePath(&dictionary_file_path);
  hyphen_dictionary_file_ = base::CreatePlatformFile(dictionary_file_path,
                                                     base::PLATFORM_FILE_READ |
                                                     base::PLATFORM_FILE_OPEN,
                                                     NULL,
                                                     NULL);
}

ShellContentBrowserClient::~ShellContentBrowserClient() {
  g_browser_client = NULL;
}

BrowserMainParts* ShellContentBrowserClient::CreateBrowserMainParts(
    const MainFunctionParams& parameters) {
  shell_browser_main_parts_ = new ShellBrowserMainParts(parameters);
  return shell_browser_main_parts_;
}

void ShellContentBrowserClient::RenderProcessHostCreated(
    RenderProcessHost* host) {
  if (!CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree))
    return;
  host->GetChannel()->AddFilter(new ShellMessageFilter(
      host->GetID(),
      BrowserContext::GetDefaultStoragePartition(browser_context())
          ->GetDatabaseTracker(),
      BrowserContext::GetDefaultStoragePartition(browser_context())
          ->GetQuotaManager()));
  host->Send(new ShellViewMsg_SetWebKitSourceDir(webkit_source_dir_));
  registrar_.Add(this,
                 NOTIFICATION_RENDERER_PROCESS_CREATED,
                 Source<RenderProcessHost>(host));
}

net::URLRequestContextGetter* ShellContentBrowserClient::CreateRequestContext(
    BrowserContext* content_browser_context,
    ProtocolHandlerMap* protocol_handlers) {
  ShellBrowserContext* shell_browser_context =
      ShellBrowserContextForBrowserContext(content_browser_context);
  return shell_browser_context->CreateRequestContext(protocol_handlers);
}

net::URLRequestContextGetter*
ShellContentBrowserClient::CreateRequestContextForStoragePartition(
    BrowserContext* content_browser_context,
    const base::FilePath& partition_path,
    bool in_memory,
    ProtocolHandlerMap* protocol_handlers) {
  ShellBrowserContext* shell_browser_context =
      ShellBrowserContextForBrowserContext(content_browser_context);
  return shell_browser_context->CreateRequestContextForStoragePartition(
      partition_path, in_memory, protocol_handlers);
}

void ShellContentBrowserClient::AppendExtraCommandLineSwitches(
    CommandLine* command_line, int child_process_id) {
  if (CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree))
    command_line->AppendSwitch(switches::kDumpRenderTree);
}

void ShellContentBrowserClient::OverrideWebkitPrefs(
    RenderViewHost* render_view_host,
    const GURL& url,
    webkit_glue::WebPreferences* prefs) {
  if (!CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree))
    return;
  WebKitTestController::Get()->OverrideWebkitPrefs(prefs);
}

void ShellContentBrowserClient::ResourceDispatcherHostCreated() {
  resource_dispatcher_host_delegate_.reset(
      new ShellResourceDispatcherHostDelegate());
  ResourceDispatcherHost::Get()->SetDelegate(
      resource_dispatcher_host_delegate_.get());
}

std::string ShellContentBrowserClient::GetDefaultDownloadName() {
  return "download";
}

bool ShellContentBrowserClient::SupportsBrowserPlugin(
    content::BrowserContext* browser_context, const GURL& url) {
  return CommandLine::ForCurrentProcess()->HasSwitch(
      switches::kEnableBrowserPluginForAllViewTypes);
}

WebContentsViewDelegate* ShellContentBrowserClient::GetWebContentsViewDelegate(
    WebContents* web_contents) {
#if !defined(USE_AURA)
  return CreateShellWebContentsViewDelegate(web_contents);
#else
  return NULL;
#endif
}

QuotaPermissionContext*
ShellContentBrowserClient::CreateQuotaPermissionContext() {
  return new ShellQuotaPermissionContext();
}

#if defined(OS_ANDROID)
void ShellContentBrowserClient::GetAdditionalMappedFilesForChildProcess(
    const CommandLine& command_line,
    int child_process_id,
    std::vector<content::FileDescriptorInfo>* mappings) {
  int flags = base::PLATFORM_FILE_OPEN | base::PLATFORM_FILE_READ;
  base::FilePath pak_file;
  bool r = PathService::Get(base::DIR_ANDROID_APP_DATA, &pak_file);
  CHECK(r);
  pak_file = pak_file.Append(FILE_PATH_LITERAL("paks"));
  pak_file = pak_file.Append(FILE_PATH_LITERAL("content_shell.pak"));

  base::PlatformFile f =
      base::CreatePlatformFile(pak_file, flags, NULL, NULL);
  if (f == base::kInvalidPlatformFileValue) {
    NOTREACHED() << "Failed to open file when creating renderer process: "
                 << "content_shell.pak";
  }
  mappings->push_back(
      content::FileDescriptorInfo(kShellPakDescriptor,
                                  base::FileDescriptor(f, true)));
}
#endif

void ShellContentBrowserClient::Observe(int type,
                                        const NotificationSource& source,
                                        const NotificationDetails& details) {
  switch (type) {
    case NOTIFICATION_RENDERER_PROCESS_CREATED: {
      registrar_.Remove(this,
                        NOTIFICATION_RENDERER_PROCESS_CREATED,
                        source);
      if (hyphen_dictionary_file_ != base::kInvalidPlatformFileValue) {
        RenderProcessHost* host = Source<RenderProcessHost>(source).ptr();
        IPC::PlatformFileForTransit file = IPC::GetFileHandleForProcess(
            hyphen_dictionary_file_, host->GetHandle(), false);
        host->Send(new ShellViewMsg_LoadHyphenDictionary(file));
      }
      break;
    }

    default:
      NOTREACHED();
  }
}

ShellBrowserContext* ShellContentBrowserClient::browser_context() {
  return shell_browser_main_parts_->browser_context();
}

ShellBrowserContext*
    ShellContentBrowserClient::off_the_record_browser_context() {
  return shell_browser_main_parts_->off_the_record_browser_context();
}

AccessTokenStore* ShellContentBrowserClient::CreateAccessTokenStore() {
  return new ShellAccessTokenStore(browser_context()->GetRequestContext());
}

ShellBrowserContext*
ShellContentBrowserClient::ShellBrowserContextForBrowserContext(
    BrowserContext* content_browser_context) {
  if (content_browser_context == browser_context())
    return browser_context();
  DCHECK_EQ(content_browser_context, off_the_record_browser_context());
  return off_the_record_browser_context();
}

}  // namespace content
