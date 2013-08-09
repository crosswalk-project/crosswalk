// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_service.h"

#include <string>

#include "base/file_util.h"
#include "xwalk/application/browser/application_process_manager.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/common/application_file_util.h"
#include "xwalk/runtime/browser/runtime_context.h"

using xwalk::RuntimeContext;

namespace xwalk {
namespace application {

const base::FilePath::CharType kApplicationsDir[] =
    FILE_PATH_LITERAL("applications");

ApplicationService::ApplicationService(RuntimeContext* runtime_context)
    : runtime_context_(runtime_context),
      app_store_(new ApplicationStore(runtime_context)) {
}

ApplicationService::~ApplicationService() {
}

bool ApplicationService::Install(const base::FilePath& path, std::string* id) {
  if (!file_util::PathExists(path))
    return false;

  const base::FilePath data_dir =
      runtime_context_->GetPath().Append(kApplicationsDir);

  std::string error;
  scoped_refptr<Application> application =
      LoadApplication(path,
                      Manifest::COMMAND_LINE,
                      &error);
  if (!application) {
    LOG(ERROR) << "Error during application installation: " << error;
    return false;
  }

  if (app_store_->AddApplication(application)) {
    LOG(INFO) << "Installed application with id: " << application->ID()
              << " successfully.";
    *id = application->ID();
    return true;
  }

  LOG(ERROR) << "Application with id " << application->ID()
             << " couldn't be installed.";
  return false;
}

bool ApplicationService::Launch(const std::string& id) {
  scoped_refptr<const Application> application =
      app_store_->GetApplicationByID(id);
  if (!application) {
    LOG(ERROR) << "Application with id " << id << " haven't installed.";
    return false;
  }

  return runtime_context_->GetApplicationSystem()->
      process_manager()->LaunchApplication(runtime_context_,
                                           application.get());
}

bool ApplicationService::Launch(const base::FilePath& path) {
  if (!file_util::DirectoryExists(path))
    return false;

  std::string error;
  scoped_refptr<const Application> application =
      LoadApplication(path, Manifest::COMMAND_LINE, &error);

  if (!application) {
    LOG(ERROR) << "Error during launch application: " << error;
    return false;
  }
  return runtime_context_->GetApplicationSystem()->
      process_manager()->LaunchApplication(runtime_context_,
                                           application.get());
}

}  // namespace application
}  // namespace xwalk
