// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/app/xwalk_main_delegate.h"

#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "content/public/browser/browser_main_runner.h"
#include "content/public/common/content_switches.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"
#include "xwalk/extensions/common/xwalk_extension_switches.h"
#include "xwalk/extensions/extension_process/xwalk_extension_process_main.h"
#include "xwalk/runtime/browser/xwalk_content_browser_client.h"
#include "xwalk/runtime/browser/ui/taskbar_util.h"
#include "xwalk/runtime/common/paths_mac.h"
#include "xwalk/runtime/common/xwalk_paths.h"
#include "xwalk/runtime/renderer/xwalk_content_renderer_client.h"

#if defined(OS_TIZEN_MOBILE)
#include "xwalk/runtime/renderer/tizen/xwalk_content_renderer_client_tizen.h"
#endif

namespace xwalk {

XWalkMainDelegate::XWalkMainDelegate()
    : content_client_(new XWalkContentClient) {
}

XWalkMainDelegate::~XWalkMainDelegate() {
  browser_client_.reset();
  renderer_client_.reset();
  content_client_.reset();
}

bool XWalkMainDelegate::BasicStartupComplete(int* exit_code) {
  logging::LoggingSettings loggingSettings;
  loggingSettings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG;
  logging::InitLogging(loggingSettings);
  SetContentClient(content_client_.get());
#if defined(OS_WIN)
  CommandLine* command_line = CommandLine::ForCurrentProcess();
  std::string process_type =
          command_line->GetSwitchValueASCII(switches::kProcessType);
  // Only set the id for browser process
  if (process_type.empty())
    SetTaskbarGroupIdForProcess();
#endif
  return false;
}

void XWalkMainDelegate::PreSandboxStartup() {
#if defined(OS_MACOSX)
  OverrideFrameworkBundlePath();
  OverrideChildProcessPath();
#endif  // OS_MACOSX

  RegisterPathProvider();
  InitializeResourceBundle();
}

int XWalkMainDelegate::RunProcess(const std::string& process_type,
    const content::MainFunctionParams& main_function_params) {
  if (process_type == switches::kXWalkExtensionProcess)
    return XWalkExtensionProcessMain(main_function_params);
  // Tell content to use default process main entries by returning -1.
  return -1;
}

// static
void XWalkMainDelegate::InitializeResourceBundle() {
  base::FilePath pak_file;
#if defined(OS_MACOSX)
  pak_file = GetResourcesPakFilePath();
#else
  base::FilePath pak_dir;
  PathService::Get(base::DIR_MODULE, &pak_dir);
  DCHECK(!pak_dir.empty());

  pak_file = pak_dir.Append(FILE_PATH_LITERAL("xwalk.pak"));
#endif
  ui::ResourceBundle::InitSharedInstanceWithPakPath(pak_file);
}

content::ContentBrowserClient* XWalkMainDelegate::CreateContentBrowserClient() {
  browser_client_.reset(new XWalkContentBrowserClient);
  return browser_client_.get();
}

content::ContentRendererClient*
    XWalkMainDelegate::CreateContentRendererClient() {
#if defined(OS_TIZEN_MOBILE)
  renderer_client_.reset(new XWalkContentRendererClientTizen());
#else
  renderer_client_.reset(new XWalkContentRendererClient());
#endif
  return renderer_client_.get();
}

}  // namespace xwalk
