// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_system.h"

#include <string>
#include "base/command_line.h"
#include "base/file_util.h"
#include "net/base/net_util.h"
#include "xwalk/application/browser/application.h"
#include "xwalk/application/browser/application_event_manager.h"
#include "xwalk/application/browser/application_service.h"
#include "xwalk/application/common/event_names.h"
#include "xwalk/runtime/browser/runtime_context.h"
#include "xwalk/runtime/common/xwalk_switches.h"

#if defined(OS_LINUX)
#include "xwalk/application/browser/application_system_linux.h"
#endif

namespace xwalk {
namespace application {

ApplicationSystem::ApplicationSystem(RuntimeContext* runtime_context)
  : runtime_context_(runtime_context),
    application_service_(new ApplicationService(runtime_context)),
    event_manager_(new ApplicationEventManager(this)) {}

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
    const CommandLine& cmd_line, const GURL& url) {
  if (cmd_line.HasSwitch(switches::kListApplications)) {
    const ApplicationData::ApplicationDataMap& apps =
        application_service_->GetInstalledApplications();
    LOG(INFO) << "Application ID                       Application Name";
    LOG(INFO) << "-----------------------------------------------------";
    ApplicationData::ApplicationDataMap::const_iterator it;
    for (it = apps.begin(); it != apps.end(); ++it)
      LOG(INFO) << it->first << "     " << it->second->Name();
    LOG(INFO) << "-----------------------------------------------------";
    return true;
  }

  if (cmd_line.HasSwitch(switches::kUninstall)) {
    const CommandLine::StringVector& args = cmd_line.GetArgs();
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
  if (ApplicationData::IsIDValid(command_name)) {
    *run_default_message_loop = application_service_->Launch(command_name);
    SendOnLaunchedEvent();
    return true;
  }
#endif

  if (!url.SchemeIsFile())
    return false;

  // Handles raw app_id passed as first non-switch argument.
  const CommandLine::StringVector& args = cmd_line.GetArgs();
  if (!args.empty()) {
    std::string app_id = std::string(args[0].begin(), args[0].end());
    if (ApplicationData::IsIDValid(app_id)) {
        *run_default_message_loop = application_service_->Launch(app_id);
        SendOnLaunchedEvent();
        return true;
    }
  }

  // Handles local directory.
  base::FilePath path;
  if (net::FileURLToFilePath(url, &path) && base::DirectoryExists(path)) {
    *run_default_message_loop = application_service_->Launch(path);
    SendOnLaunchedEvent();
    return true;
  }

  return false;
}

void ApplicationSystem::SendOnLaunchedEvent() {
  scoped_refptr<Event> event = Event::CreateEvent(
      kOnLaunched, scoped_ptr<base::ListValue>(new base::ListValue));
  DCHECK(application_service_->GetActiveApplication());
  event_manager_->SendEvent(
      application_service_->GetActiveApplication()->data()->ID(), event);
}

bool ApplicationSystem::IsRunningAsService() const {
  return false;
}

}  // namespace application
}  // namespace xwalk
