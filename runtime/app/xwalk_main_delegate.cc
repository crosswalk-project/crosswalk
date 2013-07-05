// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/runtime/app/xwalk_main_delegate.h"

#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "cameo/runtime/browser/cameo_content_browser_client.h"
#include "cameo/runtime/browser/ui/taskbar_util.h"
#include "cameo/runtime/common/xwalk_paths.h"
#include "cameo/runtime/renderer/cameo_content_renderer_client.h"
#include "content/public/browser/browser_main_runner.h"
#include "content/public/common/content_switches.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"

namespace cameo {

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
#endif
  return false;
}

void XWalkMainDelegate::PreSandboxStartup() {
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
  base::FilePath pak_file, pak_dir;
  PathService::Get(base::DIR_MODULE, &pak_dir);
  DCHECK(!pak_dir.empty());

  pak_file = pak_dir.Append(FILE_PATH_LITERAL("cameo.pak"));
  ui::ResourceBundle::InitSharedInstanceWithPakPath(pak_file);
}

content::ContentBrowserClient* XWalkMainDelegate::CreateContentBrowserClient() {
  browser_client_.reset(new CameoContentBrowserClient);
  return browser_client_.get();
}

content::ContentRendererClient*
    XWalkMainDelegate::CreateContentRendererClient() {
  renderer_client_.reset(new CameoContentRendererClient);
  return renderer_client_.get();
}

}  // namespace cameo
