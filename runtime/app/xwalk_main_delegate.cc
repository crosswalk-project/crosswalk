// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/app/xwalk_main_delegate.h"

#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "xwalk/runtime/browser/xwalk_content_browser_client.h"
#include "xwalk/runtime/browser/ui/taskbar_util.h"
#include "xwalk/runtime/common/paths_mac.h"
#include "xwalk/runtime/common/xwalk_paths.h"
#include "xwalk/runtime/renderer/xwalk_content_renderer_client.h"
#include "content/public/browser/browser_main_runner.h"
#include "content/public/common/content_switches.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"

#if defined(USE_AURA)
#include "xwalk/runtime/browser/ui/desktop_root_window_host_xwalk.h"
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
  SetContentClient(content_client_.get());
#if defined(OS_WIN)
  CommandLine* command_line = CommandLine::ForCurrentProcess();
  std::string process_type =
          command_line->GetSwitchValueASCII(switches::kProcessType);
  // Only set the id for browser process
  if (process_type.empty())
    SetTaskbarGroupIdForProcess();
#elif defined(USE_AURA)
  views::DesktopRootWindowHost::InitDesktopRootWindowHostFactory(
      views::DesktopRootWindowHostXWalk::Create);
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
  renderer_client_.reset(new XWalkContentRendererClient);
  return renderer_client_.get();
}

}  // namespace xwalk
