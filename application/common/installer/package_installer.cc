// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/installer/package_installer.h"

#include <sys/types.h>
#include <algorithm>
#include <map>
#include <string>

#include "base/file_util.h"
#include "base/files/file_enumerator.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/command_line.h"
#include "base/process/launch.h"
#include "base/version.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/application_file_util.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/permission_policy_manager.h"
#include "xwalk/application/common/application_storage.h"
#include "xwalk/application/common/id_util.h"
#include "xwalk/application/common/installer/tizen/packageinfo_constants.h"
#include "xwalk/runtime/common/xwalk_paths.h"

#if defined(OS_TIZEN)
#include "xwalk/application/common/installer/package_installer_tizen.h"
#endif

namespace xwalk {
namespace application {

namespace {

bool CopyDirectoryContents(const base::FilePath& from,
    const base::FilePath& to) {
  base::FileEnumerator iter(from, false,
      base::FileEnumerator::FILES | base::FileEnumerator::DIRECTORIES);
  for (base::FilePath path = iter.Next(); !path.empty(); path = iter.Next()) {
    if (iter.GetInfo().IsDirectory()) {
      if (!base::CopyDirectory(path, to, true))
        return false;
    } else if (!base::CopyFile(path, to.Append(path.BaseName()))) {
        return false;
    }
  }

  return true;
}

const base::FilePath::CharType kApplicationsDir[] =
    FILE_PATH_LITERAL("applications");

const base::FilePath::CharType kInstallTempDir[] =
    FILE_PATH_LITERAL("install_temp");

const base::FilePath::CharType kUpdateTempDir[] =
    FILE_PATH_LITERAL("update_temp");
}  // namespace

PackageInstaller::PackageInstaller(ApplicationStorage* storage)
    : storage_(storage) {
}

PackageInstaller::~PackageInstaller() {
}

scoped_ptr<PackageInstaller> PackageInstaller::Create(
    ApplicationStorage* storage) {
#if defined(OS_TIZEN)
  return scoped_ptr<PackageInstaller>(new PackageInstallerTizen(storage));
#else
  return scoped_ptr<PackageInstaller>(new PackageInstaller(storage));
#endif
}

void PackageInstaller::SetQuiet(bool quiet) {
}

void PackageInstaller::SetInstallationKey(const std::string& key) {
}

std::string PackageInstaller::PrepareUninstallationID(const std::string& id) {
  return id;
}

bool PackageInstaller::PlatformInstall(ApplicationData* data) {
  return true;
}

bool PackageInstaller::PlatformUninstall(ApplicationData* data) {
  return true;
}

bool PackageInstaller::PlatformUpdate(ApplicationData* updated_data) {
  return true;
}

bool PackageInstaller::Install(const base::FilePath& path, std::string* id) {
  // FIXME(leandro): Installation is not robust enough -- should any step
  // fail, it can't roll back to a consistent state.
  if (!base::PathExists(path))
    return false;

  base::FilePath data_dir, install_temp_dir;
  CHECK(PathService::Get(xwalk::DIR_DATA_PATH, &data_dir));
  install_temp_dir = data_dir.Append(kInstallTempDir);
  data_dir = data_dir.Append(kApplicationsDir);

  // Make sure the kApplicationsDir exists under data_path, otherwise,
  // the installation will always fail because of moving application
  // resources into an invalid directory.
  if (!base::DirectoryExists(data_dir) &&
      !base::CreateDirectory(data_dir))
    return false;

  if (!base::DirectoryExists(install_temp_dir) &&
      !base::CreateDirectory(install_temp_dir))
    return false;

  std::string app_id;
  base::FilePath unpacked_dir;
  scoped_ptr<Package> package;
  FileDeleter tmp_path(install_temp_dir.Append(path.BaseName()), false);
  if (!base::DirectoryExists(path)) {
    if (tmp_path.path() != path &&
        !base::CopyFile(path, tmp_path.path()))
      return false;
    package = Package::Create(tmp_path.path());
    if (!package->IsValid())
      return false;
    package->ExtractToTemporaryDir(&unpacked_dir);
    app_id = package->Id();
  } else {
    unpacked_dir = path;
  }

  std::string error;
  scoped_refptr<ApplicationData> app_data = LoadApplication(
      unpacked_dir, app_id, ApplicationData::LOCAL_DIRECTORY,
      package->type(), &error);
  if (!app_data) {
    LOG(ERROR) << "Error during application installation: " << error;
    return false;
  }

  // FIXME: Probably should be removed, as we should not handle permissions
  // inside XWalk.
  PermissionPolicyManager permission_policy_handler;
  if (!permission_policy_handler.
      InitApplicationPermission(app_data)) {
    LOG(ERROR) << "Application permission data is invalid";
    return false;
  }

  if (storage_->Contains(app_data->ID())) {
    *id = app_data->ID();
    LOG(INFO) << "Already installed: " << *id;
    return false;
  }

  base::FilePath app_dir = data_dir.AppendASCII(app_data->ID());
  if (base::DirectoryExists(app_dir)) {
    if (!base::DeleteFile(app_dir, true))
      return false;
  }
  if (!package) {
    if (!base::CreateDirectory(app_dir))
      return false;
    if (!CopyDirectoryContents(unpacked_dir, app_dir))
      return false;
  } else {
    if (!base::Move(unpacked_dir, app_dir))
      return false;
  }

  app_data->SetPath(app_dir);

  if (!storage_->AddApplication(app_data)) {
    LOG(ERROR) << "Application with id " << app_data->ID()
               << " couldn't be installed due to a Storage error";
    base::DeleteFile(app_dir, true);
    return false;
  }

  if (!PlatformInstall(app_data)) {
    LOG(ERROR) << "Application with id " << app_data->ID()
               << " couldn't be installed due to a platform error";
    storage_->RemoveApplication(app_data->ID());
    base::DeleteFile(app_dir, true);
    return false;
  }

  LOG(INFO) << "Installed application with id: " << app_data->ID()
            << "to" << app_dir.MaybeAsASCII() << " successfully.";
  *id = app_data->ID();

  return true;
}

bool PackageInstaller::Update(const std::string& app_id,
                              const base::FilePath& path) {
  if (!IsValidApplicationID(app_id)) {
    LOG(ERROR) << "The given application id " << app_id << " is invalid.";
    return false;
  }

  if (!base::PathExists(path)) {
    LOG(ERROR) << "The XPK/WGT package file " << path.value() << " is invalid.";
    return false;
  }

  if (base::DirectoryExists(path)) {
    LOG(WARNING) << "Cannot update an unpacked XPK/WGT package.";
    return false;
  }

  base::FilePath unpacked_dir, update_temp_dir;
  CHECK(PathService::Get(xwalk::DIR_DATA_PATH, &update_temp_dir));
  update_temp_dir = update_temp_dir.Append(kUpdateTempDir);
  if (!base::DirectoryExists(update_temp_dir) &&
      !base::CreateDirectory(update_temp_dir))
    return false;

  FileDeleter tmp_path(update_temp_dir.Append(path.BaseName()), false);
  if (tmp_path.path() != path &&
      !base::CopyFile(path, tmp_path.path()))
    return false;

  scoped_ptr<Package> package = Package::Create(tmp_path.path());
  if (!package) {
    LOG(ERROR) << "XPK/WGT file is invalid.";
    return false;
  }

  if (app_id.compare(package->Id()) != 0) {
    LOG(ERROR) << "The XPK/WGT file is invalid, the application id is not the"
               << "same as the installed application has.";
    return false;
  }

  if (!package->ExtractToTemporaryDir(&unpacked_dir))
    return false;

  std::string error;
  scoped_refptr<ApplicationData> new_app_data =
      LoadApplication(unpacked_dir,
                      app_id,
                      ApplicationData::TEMP_DIRECTORY,
                      package->type(),
                      &error);
  if (!new_app_data) {
    LOG(ERROR) << "An error occurred during application updating: " << error;
    return false;
  }

  scoped_refptr<ApplicationData> old_app_data =
      storage_->GetApplicationData(app_id);
  if (!old_app_data) {
    LOG(INFO) << "Application haven't installed yet: " << app_id;
    return false;
  }

  if (
#if defined(OS_TIZEN)
      // For Tizen WGT package, downgrade to a lower version or reinstall
      // is permitted when using Tizen WRT, Crosswalk runtime need to follow
      // this behavior on Tizen platform.
      package->type() != Package::WGT &&
#endif
      old_app_data->Version()->CompareTo(
          *(new_app_data->Version())) >= 0) {
    LOG(INFO) << "The version number of new XPK/WGT package "
                 "should be higher than "
              << old_app_data->VersionString();
    return false;
  }

  const base::FilePath& app_dir = old_app_data->Path();
  const base::FilePath tmp_dir(app_dir.value()
                               + FILE_PATH_LITERAL(".tmp"));

  if (!base::Move(app_dir, tmp_dir) ||
      !base::Move(unpacked_dir, app_dir))
    return false;

  new_app_data = LoadApplication(app_dir,
                                    app_id,
                                    ApplicationData::LOCAL_DIRECTORY,
                                    package->type(),
                                    &error);
  if (!new_app_data) {
    LOG(ERROR) << "Error during loading new package: " << error;
    base::DeleteFile(app_dir, true);
    base::Move(tmp_dir, app_dir);
    return false;
  }

  if (!storage_->UpdateApplication(new_app_data)) {
    LOG(ERROR) << "Fail to update application, roll back to the old one.";
    base::DeleteFile(app_dir, true);
    base::Move(tmp_dir, app_dir);
    return false;
  }

  if (!PlatformUpdate(new_app_data)) {
    LOG(ERROR) << "Fail to update application, roll back to the old one.";
    base::DeleteFile(app_dir, true);
    if (!storage_->UpdateApplication(old_app_data)) {
      LOG(ERROR) << "Fail to revert old application info, "
                 << "remove the application as a last resort.";
      storage_->RemoveApplication(old_app_data->ID());
      base::DeleteFile(tmp_dir, true);
      return false;
    }
    base::Move(tmp_dir, app_dir);
    return false;
  }

  base::DeleteFile(tmp_dir, true);

  return true;
}

bool PackageInstaller::Uninstall(const std::string& id) {
  std::string app_id = PrepareUninstallationID(id);

  if (!IsValidApplicationID(app_id)) {
    LOG(ERROR) << "The given application id " << app_id << " is invalid.";
    return false;
  }

  bool result = true;
  scoped_refptr<ApplicationData> app_data =
      storage_->GetApplicationData(app_id);
  if (!app_data) {
    LOG(ERROR) << "Failed to find application with id " << app_id
               << " among the installed ones.";
    result = false;
  }

  if (!storage_->RemoveApplication(app_id)) {
    LOG(ERROR) << "Cannot uninstall application with id " << app_id
               << "; application is not installed.";
    result = false;
  }

  base::FilePath resources;
  CHECK(PathService::Get(xwalk::DIR_DATA_PATH, &resources));
  resources = resources.Append(kApplicationsDir).AppendASCII(app_id);
  if (base::DirectoryExists(resources) &&
      !base::DeleteFile(resources, true)) {
    LOG(ERROR) << "Error occurred while trying to remove application with id "
               << app_id << "; Cannot remove all resources.";
    result = false;
  }

  if (!PlatformUninstall(app_data))
    result = false;

  return result;
}

void PackageInstaller::ContinueUnfinishedTasks() {
  base::FilePath config_dir;
  CHECK(PathService::Get(xwalk::DIR_DATA_PATH, &config_dir));

  base::FilePath install_temp_dir = config_dir.Append(kInstallTempDir),
      update_temp_dir = config_dir.Append(kUpdateTempDir);
  FileDeleter install_cleaner(install_temp_dir, true),
      update_cleaner(update_temp_dir, true);

  if (base::DirectoryExists(install_temp_dir)) {
    base::FileEnumerator install_iter(
        install_temp_dir, false, base::FileEnumerator::FILES);
    for (base::FilePath file = install_iter.Next();
         !file.empty(); file = install_iter.Next()) {
      std::string app_id;
      Install(file, &app_id);
    }
  }

  if (base::DirectoryExists(update_temp_dir)) {
    base::FileEnumerator update_iter(
        update_temp_dir, false, base::FileEnumerator::FILES);
    for (base::FilePath file = update_iter.Next();
         !file.empty(); file = update_iter.Next()) {
      std::string app_id;
      if (!Install(file, &app_id) && storage_->Contains(app_id)) {
        LOG(INFO) << "trying to update %s" << app_id;
        Update(app_id, file);
      }
    }
  }
}

}  // namespace application
}  // namespace xwalk
