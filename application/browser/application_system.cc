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

namespace xwalk {
namespace application {

ApplicationSystem::ApplicationSystem(XWalkBrowserContext* browser_context)
  : browser_context_(browser_context),
    application_service_(ApplicationService::Create(
        browser_context)) {}

ApplicationSystem::~ApplicationSystem() {
}

// static
std::unique_ptr<ApplicationSystem> ApplicationSystem::Create(
    XWalkBrowserContext* browser_context) {
  std::unique_ptr<ApplicationSystem> app_system;
  app_system.reset(new ApplicationSystem(browser_context));
  return app_system;
}

bool ApplicationSystem::LaunchFromCommandLine(
    const base::CommandLine& cmd_line, const GURL& url) {
  if (!url.is_valid())
    return false;

  base::FilePath path;
  Application* app = nullptr;
  bool is_local = url.SchemeIsFile() && net::FileURLToFilePath(url, &path);
  if (!is_local) {  // Handles external URL.
    app = application_service_->LaunchHostedURL(url);
    return !!app;
  }

  if (!base::PathExists(path))
    return false;

  if (path.MatchesExtension(FILE_PATH_LITERAL(".xpk")) ||
      path.MatchesExtension(FILE_PATH_LITERAL(".wgt"))) {
    app = application_service_->LaunchFromPackagePath(path);
    return !!app;
  }

  if (path.MatchesExtension(FILE_PATH_LITERAL(".json"))) {
    app = application_service_->LaunchFromManifestPath(
        path, Manifest::TYPE_MANIFEST);
    return !!app;
  }

  if (path.MatchesExtension(FILE_PATH_LITERAL(".xml"))) {
    app = application_service_->LaunchFromManifestPath(
        path, Manifest::TYPE_WIDGET);
    return !!app;
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
  // Register the widget extension only when the application is widget format.
  if (application->data()->manifest_type() == Manifest::TYPE_WIDGET)
      extensions->push_back(new ApplicationWidgetExtension(application));
}

}  // namespace application
}  // namespace xwalk
