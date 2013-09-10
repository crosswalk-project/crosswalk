// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/application_service.h"

#include <string>

#include "base/file_util.h"
#include "xwalk/application/browser/application_process_manager.h"
#include "xwalk/application/browser/application_system.h"
#include "xwalk/application/browser/installer/xpk_extractor.h"
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

  // Make sure the kApplicationsDir exists under data_path, otherwise,
  // the installation will always fail because of moving application
  // resources into an invalid directory.
  if (!file_util::DirectoryExists(data_dir) &&
      !file_util::CreateDirectory(data_dir))
    return false;

  base::FilePath unpacked_dir;
  std::string app_id;
  if (!file_util::DirectoryExists(path)) {
    scoped_refptr<XPKExtractor> extractor = XPKExtractor::Create(path);
    if (extractor)
      app_id = extractor->GetPackageID();

    if (app_id.empty()) {
      LOG(ERROR) << "XPK file is invalid.";
      return false;
    }

    if (app_store_->Contains(app_id)) {
      *id = app_id;
      LOG(INFO) << "Already installed: " << app_id;
      return true;
    }

    base::FilePath temp_dir;
    extractor->Extract(&temp_dir);
    unpacked_dir = data_dir.AppendASCII(app_id);
    if (file_util::DirectoryExists(unpacked_dir) &&
        !file_util::Delete(unpacked_dir, true))
      return false;
    if (!file_util::Move(temp_dir, unpacked_dir))
      return false;
  } else {
    unpacked_dir = path;
  }

  std::string error;
  scoped_refptr<Application> application =
      LoadApplication(unpacked_dir,
                      app_id,
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

bool ApplicationService::Uninstall(const std::string& id) {
  if (!app_store_->RemoveApplication(id)) {
    LOG(ERROR) << "Cannot uninstall application with id " << id
               << "; application is not installed.";
    return false;
  }

  const base::FilePath resources =
      runtime_context_->GetPath().Append(kApplicationsDir).AppendASCII(id);
  if (file_util::DirectoryExists(resources) &&
      !file_util::Delete(resources, true)) {
    LOG(ERROR) << "Error occurred while trying to remove application with id "
               << id << "; Cannot remove all resources.";
    return false;
  }
  return true;
}

bool ApplicationService::Launch(const std::string& id) {
  scoped_refptr<const Application> application =
      app_store_->GetApplicationByID(id);
  if (!application) {
    LOG(ERROR) << "Application with id " << id << " haven't installed.";
    return false;
  }

  application_ = application;
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

  application_ = application;
  return runtime_context_->GetApplicationSystem()->
      process_manager()->LaunchApplication(runtime_context_,
                                           application.get());
}

const Application* ApplicationService::GetRunningApplication() const {
  return application_.get();
}

}  // namespace application
}  // namespace xwalk
