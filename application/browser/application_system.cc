// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_system.h"

#include <string>
#include "base/command_line.h"
#include "base/file_util.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/common/content_switches.h"
#include "net/base/filename_util.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/common/application_storage.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/id_util.h"
#include "xwalk/application/extension/application_runtime_extension.h"
#include "xwalk/application/extension/application_widget_extension.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/common/xwalk_switches.h"

#if defined(OS_LINUX)
#include "xwalk/application/browser/application_system_linux.h"
#endif

namespace xwalk {
namespace application {

ApplicationSystem::ApplicationSystem(RuntimeContext* runtime_context)
  : runtime_context_(runtime_context),
    application_storage_(new ApplicationStorage(runtime_context->GetPath())),
    application_service_(new ApplicationService(
        runtime_context,
        application_storage_.get())) {}

ApplicationSystem::~ApplicationSystem() {
}

// static
scoped_ptr<ApplicationSystem> ApplicationSystem::Create(
    RuntimeContext* runtime_context) {
  scoped_ptr<ApplicationSystem> app_system;
#if defined(OS_LINUX)
  app_system.reset(new ApplicationSystemLinux(runtime_context));
#else
  app_system.reset(new ApplicationSystem(runtime_context));
#endif
  return app_system.Pass();
}

template <typename T>
bool ApplicationSystem::LaunchWithCommandLineParam(
    const T& param, const base::CommandLine& cmd_line) {
  Application::LaunchParams launch_params;
  launch_params.force_fullscreen = cmd_line.HasSwitch(switches::kFullscreen);
  launch_params.remote_debugging =
      cmd_line.HasSwitch(switches::kRemoteDebuggingPort);

  return application_service_->Launch(param, launch_params);
}

// Launch an application created from arbitrary url.
// FIXME: This application should have the same strict permissions
// as common browser apps.
template <>
bool ApplicationSystem::LaunchWithCommandLineParam<GURL>(
    const GURL& url, const base::CommandLine& cmd_line) {
  std::string error;
  scoped_refptr<ApplicationData> application_data =
      ApplicationData::Create(url, &error);
  if (!application_data) {
    LOG(ERROR) << "Error occurred while trying to launch application: "
               << error;
    return false;
  }

  Application::LaunchParams launch_params;
  launch_params.force_fullscreen = cmd_line.HasSwitch(switches::kFullscreen);
  launch_params.entry_points = Application::StartURLKey;
  launch_params.remote_debugging =
      cmd_line.HasSwitch(switches::kRemoteDebuggingPort);

  return !!application_service_->Launch(application_data, launch_params);
}

bool ApplicationSystem::LaunchFromCommandLine(
    const base::CommandLine& cmd_line, const GURL& url,
    bool& run_default_message_loop) { // NOLINT

  // Handles raw app_id passed as first non-switch argument.
  const base::CommandLine::StringVector& args = cmd_line.GetArgs();
  if (!args.empty()) {
    std::string app_id = std::string(args[0].begin(), args[0].end());
    if (IsValidApplicationID(app_id)) {
      run_default_message_loop = LaunchWithCommandLineParam(app_id, cmd_line);
      return true;
    }
  }

  if (!url.is_valid())
    return false;

  base::FilePath path;
  if (url.SchemeIsFile() &&
      net::FileURLToFilePath(url, &path) &&
      base::PathExists(path)) {  // Handles local path.
    run_default_message_loop = LaunchWithCommandLineParam(path, cmd_line);
  } else {  // Handles external URL.
    run_default_message_loop = LaunchWithCommandLineParam(url, cmd_line);
  }

  return true;
}

void ApplicationSystem::CreateExtensions(
    content::RenderProcessHost* host,
    extensions::XWalkExtensionVector* extensions) {
  Application* application =
    application_service_->GetApplicationByRenderHostID(host->GetID());
  if (!application)
    return;  // We might be in browser mode.

  extensions->push_back(new ApplicationRuntimeExtension(application));
  extensions->push_back(new ApplicationWidgetExtension(application));
}

}  // namespace application
}  // namespace xwalk
