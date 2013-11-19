// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_system.h"

#include <string>
#include "base/command_line.h"
#include "base/file_util.h"
#include "net/base/net_util.h"
#include "xwalk/application/browser/application_event_manager.h"
#include "xwalk/application/browser/application_process_manager.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/browser/application_service_provider.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/common/xwalk_switches.h"

using xwalk::RuntimeContext;

namespace xwalk {
namespace application {

ApplicationSystem::ApplicationSystem(RuntimeContext* runtime_context)
  : runtime_context_(runtime_context),
    process_manager_(new ApplicationProcessManager(runtime_context)),
    application_service_(new ApplicationService(runtime_context)),
    event_manager_(new ApplicationEventManager(this)) {
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(switches::kXWalkRunAsService)) {
    service_provider_ =
        ApplicationServiceProvider::Create(application_service_.get());
  }
}

ApplicationSystem::~ApplicationSystem() {
}

bool ApplicationSystem::HandleApplicationManagementCommands(
    const CommandLine& cmd_line, const GURL& url) {
  if (cmd_line.HasSwitch(switches::kListApplications)) {
    ApplicationStore::ApplicationMap* apps =
        application_service_->GetInstalledApplications();
    LOG(INFO) << "Application ID                       Application Name";
    LOG(INFO) << "-----------------------------------------------------";
    ApplicationStore::ApplicationMapIterator it;
    for (it = apps->begin(); it != apps->end(); ++it)
      LOG(INFO) << it->first << "     " << it->second->Name();
    LOG(INFO) << "-----------------------------------------------------";
    return true;
  }

  if (cmd_line.HasSwitch(switches::kUninstall)) {
    const CommandLine::StringVector& args = cmd_line.GetArgs();
    if (args.empty())
      return false;

    std::string app_id = std::string(args[0].begin(), args[0].end());
    if (!Application::IsIDValid(app_id))
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
    if (application_service_->Install(path, &app_id))
      LOG(INFO) << "[OK] Application installed: " << app_id;
    else
      LOG(ERROR) << "[ERR] Application install failure: " << path.value();
    return true;
  }

  return false;
}

bool ApplicationSystem::LaunchFromCommandLine(
    const CommandLine& cmd_line, const GURL& url,
    bool* run_default_message_loop) {
  // On Tizen, applications are launched by a symbolic link named like the
  // application ID.
  // FIXME(cmarcelo): Remove when we move to a separate launcher on Tizen.
#if defined(OS_TIZEN_MOBILE)
  std::string command_name = cmd_line.GetProgram().BaseName().MaybeAsASCII();
  if (Application::IsIDValid(command_name)) {
    *run_default_message_loop = application_service_->Launch(command_name);
    return true;
  }
#endif

  if (!url.SchemeIsFile())
    return false;

  // Handles raw app_id passed as first non-switch argument.
  const CommandLine::StringVector& args = cmd_line.GetArgs();
  if (!args.empty()) {
    std::string app_id = std::string(args[0].begin(), args[0].end());
    if (Application::IsIDValid(app_id)) {
        *run_default_message_loop = application_service_->Launch(app_id);
        return true;
    }
  }

  // Handles local directory.
  base::FilePath path;
  if (net::FileURLToFilePath(url, &path) && base::DirectoryExists(path)) {
    *run_default_message_loop = application_service_->Launch(path);
    return true;
  }

  return false;
}

}  // namespace application
}  // namespace xwalk
