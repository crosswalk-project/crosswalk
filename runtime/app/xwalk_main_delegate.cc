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
#include "xwalk/runtime/browser/xwalk_runner.h"
#include "xwalk/runtime/browser/ui/taskbar_util.h"
#include "xwalk/runtime/common/paths_mac.h"
#include "xwalk/runtime/common/xwalk_paths.h"
#include "xwalk/runtime/renderer/xwalk_content_renderer_client.h"

#if !defined(DISABLE_NACL) && defined(OS_LINUX)
#include "components/nacl/common/nacl_paths.h"
#include "components/nacl/zygote/nacl_fork_delegate_linux.h"
#endif

namespace xwalk {

XWalkMainDelegate::XWalkMainDelegate()
    : content_client_(new XWalkContentClient) {
}

XWalkMainDelegate::~XWalkMainDelegate() {}

bool XWalkMainDelegate::BasicStartupComplete(int* exit_code) {
  logging::LoggingSettings loggingSettings;
  loggingSettings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG;
  logging::InitLogging(loggingSettings);
  SetContentClient(content_client_.get());
#if defined(OS_MACOSX)
  OverrideFrameworkBundlePath();
  OverrideChildProcessPath();
#elif defined(OS_WIN)
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  std::string process_type =
          command_line->GetSwitchValueASCII(switches::kProcessType);
  // Only set the id for browser process
  if (process_type.empty())
    SetTaskbarGroupIdForProcess();
#endif

#if !defined(DISABLE_NACL) && defined(OS_LINUX)
  nacl::RegisterPathProvider();
#endif

  return false;
}

void XWalkMainDelegate::PreSandboxStartup() {
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

#if defined(OS_POSIX) && !defined(OS_ANDROID) && !defined(OS_MACOSX)
void XWalkMainDelegate::ZygoteStarting(
    ScopedVector<content::ZygoteForkDelegate>* delegates) {
#if !defined(DISABLE_NACL)
  nacl::AddNaClZygoteForkDelegates(delegates);
#endif
}

#endif  // defined(OS_POSIX) && !defined(OS_ANDROID)

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

#if !defined(OS_ANDROID)
  ui::ResourceBundle::InitSharedInstanceWithLocale(
      "en-US", nullptr, ui::ResourceBundle::DO_NOT_LOAD_COMMON_RESOURCES);
  ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      pak_file, ui::SCALE_FACTOR_NONE);
#else
  ui::ResourceBundle::InitSharedInstanceWithPakPath(pak_file);
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
