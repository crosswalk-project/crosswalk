// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_system.h"

#include <string>
#include "base/command_line.h"
#include "base/files/file_util.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/common/content_switches.h"
#include "net/base/filename_util.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/id_util.h"
#include "xwalk/application/extension/application_runtime_extension.h"
#include "xwalk/application/extension/application_widget_extension.h"
#include "xwalk/runtime/browser/xwalk_browser_context.h"
#include "xwalk/runtime/common/xwalk_switches.h"

#if defined(OS_LINUX)
#include "xwalk/application/browser/application_system_linux.h"
#endif

#if defined(OS_TIZEN)
#include "xwalk/application/browser/application_service_tizen.h"
#endif

namespace xwalk {
namespace application {

ApplicationSystem::ApplicationSystem(XWalkBrowserContext* browser_context)
  : browser_context_(browser_context),
    application_service_(ApplicationService::Create(
        browser_context)) {}

ApplicationSystem::~ApplicationSystem() {
}

// static
scoped_ptr<ApplicationSystem> ApplicationSystem::Create(
    XWalkBrowserContext* browser_context) {
  scoped_ptr<ApplicationSystem> app_system;
#if defined(OS_LINUX)
  app_system.reset(new ApplicationSystemLinux(browser_context));
#else
  app_system.reset(new ApplicationSystem(browser_context));
#endif
  return app_system.Pass();
}

namespace {

Application::LaunchParams launch_params(
    const base::CommandLine& cmd_line) {
  Application::LaunchParams params = {
      0,
      cmd_line.HasSwitch(switches::kFullscreen),
      cmd_line.HasSwitch(switches::kRemoteDebuggingPort)};
  return params;
}

}  // namespace

bool ApplicationSystem::LaunchFromCommandLine(
    const base::CommandLine& cmd_line, const GURL& url,
    bool& run_default_message_loop) { // NOLINT

#if defined(OS_TIZEN)
  // Handles raw app_id passed as first non-switch argument.
  const base::CommandLine::StringVector& args = cmd_line.GetArgs();
  if (!args.empty()) {
    std::string app_id = std::string(args[0].begin(), args[0].end());
    if (IsValidApplicationID(app_id)) {
      ApplicationServiceTizen* app_service_tizen =
          ToApplicationServiceTizen(application_service_.get());
      run_default_message_loop = app_service_tizen->LaunchFromAppID(
          app_id, launch_params(cmd_line));
      return true;
    }
  }
#endif
  if (!url.is_valid())
    return false;

  base::FilePath path;
  bool is_local = url.SchemeIsFile() && net::FileURLToFilePath(url, &path);
  if (!is_local) {  // Handles external URL.
    run_default_message_loop = application_service_->LaunchHostedURL(
        url, launch_params(cmd_line));
    return true;
  }

  if (!base::PathExists(path))
    return false;

  if (path.MatchesExtension(FILE_PATH_LITERAL(".xpk")) ||
      path.MatchesExtension(FILE_PATH_LITERAL(".wgt"))) {
    run_default_message_loop = application_service_->LaunchFromPackagePath(
        path, launch_params(cmd_line));
    return true;
  }

  if (path.MatchesExtension(FILE_PATH_LITERAL(".json"))) {
    run_default_message_loop = application_service_->LaunchFromManifestPath(
        path, Manifest::TYPE_MANIFEST, launch_params(cmd_line));
    return true;
  }

  if (path.MatchesExtension(FILE_PATH_LITERAL(".xml"))) {
    run_default_message_loop = application_service_->LaunchFromManifestPath(
        path, Manifest::TYPE_WIDGET, launch_params(cmd_line));
    return true;
  }

  return false;
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
