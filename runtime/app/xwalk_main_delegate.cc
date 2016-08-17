// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/app/xwalk_main_delegate.h"

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "content/public/browser/browser_main_runner.h"
#include "content/public/common/content_switches.h"
#include "ui/base/material_design/material_design_controller.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"
#include "xwalk/extensions/common/xwalk_extension_switches.h"
#include "xwalk/extensions/extension_process/xwalk_extension_process_main.h"
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/runtime/common/logging_xwalk.h"
#include "xwalk/runtime/common/paths_mac.h"
#include "xwalk/runtime/common/xwalk_paths.h"
#include "xwalk/runtime/common/xwalk_resource_delegate.h"
#include "xwalk/runtime/renderer/xwalk_content_renderer_client.h"

#if !defined(DISABLE_NACL) && defined(OS_LINUX)
#include "components/nacl/common/nacl_paths.h"
#include "components/nacl/zygote/nacl_fork_delegate_linux.h"
#endif

namespace xwalk {

namespace {

#if !defined(OS_ANDROID)
void InitLogging(const std::string& process_type) {
  logging::OldFileDeletionState file_state =
      logging::APPEND_TO_OLD_LOG_FILE;
  if (process_type.empty()) {
    file_state = logging::DELETE_OLD_LOG_FILE;
  }
  const base::CommandLine& command_line =
      *base::CommandLine::ForCurrentProcess();
  logging::InitXwalkLogging(command_line, file_state);
}
#endif

}  // namespace

XWalkMainDelegate::XWalkMainDelegate()
    : content_client_(new XWalkContentClient) {
}

XWalkMainDelegate::~XWalkMainDelegate() {}

bool XWalkMainDelegate::BasicStartupComplete(int* exit_code) {
  SetContentClient(content_client_.get());
#if defined(OS_MACOSX)
  OverrideFrameworkBundlePath();
  OverrideChildProcessPath();
#endif

#if !defined(DISABLE_NACL) && defined(OS_LINUX)
  nacl::RegisterPathProvider();
#endif

  return false;
}

void XWalkMainDelegate::PreSandboxStartup() {
  RegisterPathProvider();
  InitializeResourceBundle();

#if !defined(OS_ANDROID) && !defined(OS_WIN)
  // Android does InitLogging when library is loaded. Skip here.
  // For windows we call InitLogging when the sandbox is initialized.
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  std::string process_type =
      command_line->GetSwitchValueASCII(switches::kProcessType);
  InitLogging(process_type);
#endif

#if !defined(OS_ANDROID)
  ui::MaterialDesignController::Initialize();
#endif
}

void XWalkMainDelegate::SandboxInitialized(const std::string& process_type) {
#if defined(OS_WIN)
  InitLogging(process_type);
#endif
}

int XWalkMainDelegate::RunProcess(const std::string& process_type,
    const content::MainFunctionParams& main_function_params) {
  if (process_type == switches::kXWalkExtensionProcess)
    return XWalkExtensionProcessMain(main_function_params);
  // Tell content to use default process main entries by returning -1.
  return -1;
}

void XWalkMainDelegate::ProcessExiting(const std::string& process_type) {
#if !defined(OS_ANDROID)
  logging::CleanupXwalkLogging();
#else
  // Android doesn't use InitXwalkLogging, so we close the log file manually.
  logging::CloseLogFile();
#endif  // !defined(OS_ANDROID)
}

#if defined(OS_POSIX) && !defined(OS_ANDROID) && !defined(OS_MACOSX)
void XWalkMainDelegate::ZygoteStarting(
    ScopedVector<content::ZygoteForkDelegate>* delegates) {
#if !defined(DISABLE_NACL)
  nacl::AddNaClZygoteForkDelegates(delegates);
#endif
}

#endif  // defined(OS_POSIX) && !defined(OS_ANDROID)

void XWalkMainDelegate::InitializeResourceBundle() {
  base::FilePath pak_file;
  base::FilePath pak_dir;
#if defined(OS_MACOSX)
  pak_file = GetResourcesPakFilePath();
  pak_dir = pak_file.DirName();
#else
  PathService::Get(base::DIR_MODULE, &pak_dir);
  DCHECK(!pak_dir.empty());
#endif

#if !defined(OS_ANDROID)
  resource_delegate_.reset(new XWalkResourceDelegate());
  ui::ResourceBundle::InitSharedInstanceWithLocale(
      "en-US", resource_delegate_.get(),
      ui::ResourceBundle::DO_NOT_LOAD_COMMON_RESOURCES);
  pak_file = pak_dir.Append(FILE_PATH_LITERAL("xwalk.pak"));
  ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      pak_file, ui::SCALE_FACTOR_NONE);
  pak_file = pak_dir.Append(FILE_PATH_LITERAL("xwalk_100_percent.pak"));
  ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      pak_file, ui::SCALE_FACTOR_100P);
  pak_file = pak_dir.Append(FILE_PATH_LITERAL("xwalk_200_percent.pak"));
  ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      pak_file, ui::SCALE_FACTOR_200P);
  pak_file = pak_dir.Append(FILE_PATH_LITERAL("xwalk_300_percent.pak"));
  ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      pak_file, ui::SCALE_FACTOR_300P);
#endif
}

content::ContentBrowserClient* XWalkMainDelegate::CreateContentBrowserClient() {
  // This will only be called from the Browser Process, so it is a convenient
  // location to initialize the XWalkRunner, which is our main entry point in
  // Browser Process.
  xwalk_runner_ = XWalkRunner::Create();
  return xwalk_runner_->GetContentBrowserClient();
}

content::ContentRendererClient*
    XWalkMainDelegate::CreateContentRendererClient() {
  renderer_client_.reset(new XWalkContentRendererClient());
  return renderer_client_.get();
}

}  // namespace xwalk
