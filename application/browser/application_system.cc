// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_system.h"

#include "base/command_line.h"
#include "base/file_util.h"
#include "net/base/net_util.h"
#include "xwalk/application/browser/application_process_manager.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/common/xwalk_switches.h"

using xwalk::RuntimeContext;

namespace xwalk {
namespace application {

ApplicationSystem::ApplicationSystem(RuntimeContext* runtime_context) {
  runtime_context_ = runtime_context;
  process_manager_.reset(new ApplicationProcessManager(runtime_context));
  application_service_.reset(new ApplicationService(runtime_context));
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

}  // namespace application
}  // namespace xwalk
