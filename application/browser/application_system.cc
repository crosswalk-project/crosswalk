// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_system.h"

#include <string>
#include "base/command_line.h"
#include "base/file_util.h"
#include "content/public/browser/render_process_host.h"
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

bool ApplicationSystem::HandleApplicationManagementCommands(
    const base::CommandLine& cmd_line, const GURL& url,
    bool& run_default_message_loop) { // NOLINT
  run_default_message_loop = false;
  if (cmd_line.HasSwitch(switches::kListApplications)) {
    ApplicationData::ApplicationDataMap apps;
    application_storage_->GetInstalledApplications(apps);
    LOG(INFO) << "Application ID                       Application Name";
    LOG(INFO) << "-----------------------------------------------------";
    ApplicationData::ApplicationDataMap::const_iterator it;
    for (it = apps.begin(); it != apps.end(); ++it)
      LOG(INFO) << it->first << "     " << it->second->Name();
    LOG(INFO) << "-----------------------------------------------------";
    return true;
  }

  if (cmd_line.HasSwitch(switches::kUninstall)) {
    const base::CommandLine::StringVector& args = cmd_line.GetArgs();
    if (args.empty())
      return false;

    std::string app_id = std::string(args[0].begin(), args[0].end());
    if (!ApplicationData::IsIDValid(app_id))
      return false;

    if (application_service_->Uninstall(app_id)) {
      LOG(INFO) << "[OK] Application uninstalled successfully: " << app_id;
    } else {
      LOG(ERROR) << "[ERR] An error occurred when uninstalling application "
                 << app_id;
    }
    return true;
  }

  if (cmd_line.HasSwitch(switches::kInstall)) {
    base::FilePath path;
    if (!net::FileURLToFilePath(url, &path))
      return false;

    if (!base::PathExists(path))
      return false;

    std::string app_id;
    if (application_service_->Install(path, &app_id)) {
      LOG(INFO) << "[OK] Application installed: " << app_id;
    } else if (!app_id.empty() &&
               application_service_->Update(app_id, path)) {
      LOG(INFO) << "[OK] Application updated: " << app_id;
    } else {
      LOG(ERROR) << "[ERR] Application install/update failure: "
                 << path.value();
    }
    return true;
  }

  run_default_message_loop = true;
  return false;
}

template <typename T>
bool ApplicationSystem::LaunchWithCommandLineParam(
    const T& param, const base::CommandLine& cmd_line) {
  Application::LaunchParams launch_params;
  launch_params.force_fullscreen = cmd_line.HasSwitch(switches::kFullscreen);

  return application_service_->Launch(param, launch_params);
}

// Launch an application created from arbitrary url.
// FIXME: This application should have the same strict permissions
// as common browser apps.
template <>
bool ApplicationSystem::LaunchWithCommandLineParam<GURL>(
    const GURL& url, const base::CommandLine& cmd_line) {
  namespace keys = xwalk::application_manifest_keys;

  const std::string& url_spec = url.spec();
  DCHECK(!url_spec.empty());
  const std::string& app_id = GenerateId(url_spec);
  // FIXME: we need to handle hash collisions.
  DCHECK(!application_storage_->GetApplicationData(app_id));

  base::DictionaryValue manifest;
  // FIXME: define permissions!
  manifest.SetString(keys::kURLKey, url_spec);
  manifest.SetString(keys::kNameKey,
      "Crosswalk Hosted App [Restricted Permissions]");
  manifest.SetString(keys::kVersionKey, "0");
  manifest.SetInteger(keys::kManifestVersionKey, 1);
  std::string error;
  scoped_refptr<ApplicationData> application_data = ApplicationData::Create(
            base::FilePath(), Manifest::COMMAND_LINE, manifest, app_id, &error);
  if (!application_data) {
    LOG(ERROR) << "Error occurred while trying to launch application: "
               << error;
    return false;
  }

  Application::LaunchParams launch_params;
  launch_params.force_fullscreen = cmd_line.HasSwitch(switches::kFullscreen);
  launch_params.entry_points = Application::URLKey;

  return !!application_service_->Launch(application_data, launch_params);
}

bool ApplicationSystem::LaunchFromCommandLine(
    const base::CommandLine& cmd_line, const GURL& url,
    bool& run_default_message_loop) { // NOLINT

  // Handles raw app_id passed as first non-switch argument.
  const base::CommandLine::StringVector& args = cmd_line.GetArgs();
  if (!args.empty()) {
    std::string app_id = std::string(args[0].begin(), args[0].end());
    if (ApplicationData::IsIDValid(app_id)) {
      run_default_message_loop = LaunchWithCommandLineParam(app_id, cmd_line);
      return true;
    }
  }

  if (!url.is_valid())
    return false;

  base::FilePath path;
  if (url.SchemeIsFile() &&
      net::FileURLToFilePath(url, &path) &&
      base::DirectoryExists(path)) {  // Handles local directory.
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
