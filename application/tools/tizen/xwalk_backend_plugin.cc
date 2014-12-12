// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/tools/tizen/xwalk_backend_plugin.h"

#include <cstdlib>
#include <cstring>
#include <vector>

#include "base/files/file_util.h"
#include "base/files/file_path.h"
#include "base/files/scoped_temp_dir.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/time/time.h"
#include "base/version.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/application_file_util.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/id_util.h"
#include "xwalk/application/common/manifest_handlers/tizen_application_handler.h"
#include "xwalk/application/common/package/package.h"
#include "xwalk/application/common/tizen/package_query.h"
#include "xwalk/runtime/common/xwalk_paths.h"

using xwalk::application::Manifest;

namespace {

enum PkgmgrPluginBool {
  kPkgmgrPluginTrue = 0,
  kPkgmgrPluginFalse = -1
};

// Whole app directory size in KB
int64 CountAppTotalSize(
    scoped_refptr<xwalk::application::ApplicationData> app_data) {
  return base::ComputeDirectorySize(app_data->path()) / 1024;
}

// Data directory size in KB
int64 CountAppDataSize(
    scoped_refptr<xwalk::application::ApplicationData> app_data) {
  int64 size = 0;

  base::FilePath private_path = app_data->path().Append("private");
  size += base::ComputeDirectorySize(private_path);

  base::FilePath tmp_path = app_data->path().Append("tmp");
  size += base::ComputeDirectorySize(tmp_path);

  return size / 1024;
}

}  // namespace

PkgmgrBackendPlugin* PkgmgrBackendPlugin::GetInstance() {
  return Singleton<PkgmgrBackendPlugin,
                   PkgmgrBackendPluginTraits<PkgmgrBackendPlugin> >::get();
}

int PkgmgrBackendPlugin::DetailedInfo(
    const std::string& pkgid,
    package_manager_pkg_detail_info_t* pkg_detail_info) {

  std::string app_id = xwalk::application::PkgIdToAppId(pkgid);

  if (app_id.empty())
    return kPkgmgrPluginFalse;

  scoped_refptr<xwalk::application::ApplicationData> app_data =
      storage_->GetApplicationData(app_id);
  if (!app_data.get())
    return kPkgmgrPluginFalse;

  SaveDetailInfo(app_data, pkg_detail_info);
  return kPkgmgrPluginTrue;
}

int PkgmgrBackendPlugin::DetailedInfoPkg(
    const std::string& pkg_path,
    package_manager_pkg_detail_info_t* pkg_detail_info) {
  base::FilePath path(pkg_path);
  if (!base::PathExists(path)) {
    return kPkgmgrPluginFalse;
  }

  base::ScopedTempDir dir;
  dir.CreateUniqueTempDir();
  scoped_refptr<xwalk::application::ApplicationData> app_data =
      GetApplicationDataFromPkg(pkg_path, &dir);
  if (app_data.get() == NULL) {
    return kPkgmgrPluginFalse;
  }

  SaveDetailInfo(app_data, pkg_detail_info,
      !path.Extension().empty() ? path.Extension() : std::string("unknown"));
  return kPkgmgrPluginTrue;
}

int PkgmgrBackendPlugin::IsAppInstalled(const std::string& pkgid) {
  // this will fetch app_id if exists
  std::string app_id = xwalk::application::PkgIdToAppId(pkgid);

  if (app_id.empty())
    return kPkgmgrPluginFalse;

  // backendlib handles both xpk and wgt
  // check if plugin was loaded for given type of package
  std::string type_from_db = xwalk::application::GetPackageType(pkgid);

  return type() == type_from_db ? kPkgmgrPluginTrue : kPkgmgrPluginFalse;
}

int PkgmgrBackendPlugin::AppsList(package_manager_pkg_info_t** list,
                                  int* count) {
  *list = NULL;
  *count = 0;
  std::vector<std::string> app_ids;
  if (!storage_->GetInstalledApplicationIDs(app_ids)) {
    return kPkgmgrPluginFalse;
  }
  for (std::vector<std::string>::const_iterator citer = app_ids.begin();
       citer != app_ids.end(); ++citer) {
    scoped_refptr<xwalk::application::ApplicationData> app_data =
        storage_->GetApplicationData(*citer);
    if (app_data.get() != NULL) {
      package_manager_pkg_info_t* result =
          static_cast<package_manager_pkg_info_t*>(
              malloc(sizeof(package_manager_pkg_info_t)));
      memset(result, 0x00, sizeof(package_manager_pkg_info_t));
      SaveInfo(app_data, result);
      if (*list) {
        result->next = *list;
      }
      *list = result;
      ++*count;
    }
  }
  return kPkgmgrPluginTrue;
}

void PkgmgrBackendPlugin::SetLoadSet(pkg_plugin_set* set) {
  set_ = set;
}

std::string PkgmgrBackendPlugin::type() const {
  return std::string(set_->pkg_type);
}

PkgmgrBackendPlugin::PkgmgrBackendPlugin() {
  base::FilePath data_path;
  xwalk::RegisterPathProvider();
  PathService::Get(xwalk::DIR_DATA_PATH, &data_path);
  storage_.reset(new xwalk::application::ApplicationStorage(data_path));
}

void PkgmgrBackendPlugin::SaveInfo(
    scoped_refptr<xwalk::application::ApplicationData> app_data,
    package_manager_pkg_info_t* pkg_detail_info,
    const std::string& force_type) {
  std::string pkg_id = app_data->GetPackageID();
  if (force_type.empty())
    strncpy(pkg_detail_info->pkg_type,
            xwalk::application::GetPackageType(pkg_id).c_str(),
            PKG_TYPE_STRING_LEN_MAX - 1);
  else  // force package type
    strncpy(pkg_detail_info->pkg_type,
            force_type.c_str(),
            PKG_TYPE_STRING_LEN_MAX - 1);
  strncpy(pkg_detail_info->pkg_name, pkg_id.c_str(),
          PKG_NAME_STRING_LEN_MAX - 1);
  strncpy(pkg_detail_info->pkgid, pkg_id.c_str(),
          PKG_NAME_STRING_LEN_MAX - 1);
  if (app_data->Version() != NULL) {
    strncpy(pkg_detail_info->version, app_data->Version()->GetString().c_str(),
            PKG_VERSION_STRING_LEN_MAX - 1);
  }
}

void PkgmgrBackendPlugin::SaveDetailInfo(
    scoped_refptr<xwalk::application::ApplicationData> app_data,
    package_manager_pkg_detail_info_t* pkg_detail_info,
    const std::string& force_type) {
  std::string pkg_id = app_data->GetPackageID();
  if (force_type.empty())
    strncpy(pkg_detail_info->pkg_type,
            xwalk::application::GetPackageType(pkg_id).c_str(),
            PKG_TYPE_STRING_LEN_MAX - 1);
  else  // force package type
    strncpy(pkg_detail_info->pkg_type,
            force_type.c_str(),
            PKG_TYPE_STRING_LEN_MAX - 1);
  strncpy(pkg_detail_info->pkg_name, pkg_id.c_str(),
          PKG_NAME_STRING_LEN_MAX - 1);
  strncpy(pkg_detail_info->pkgid, pkg_id.c_str(),
          PKG_NAME_STRING_LEN_MAX - 1);
  if (app_data->Version() != NULL) {
    strncpy(pkg_detail_info->version, app_data->Version()->GetString().c_str(),
            PKG_VERSION_STRING_LEN_MAX - 1);
  }
  strncpy(pkg_detail_info->pkg_description, app_data->Description().c_str(),
          PKG_VALUE_STRING_LEN_MAX - 1);

  // xpk do not have this key in manifest
  if (app_data->manifest_type() == Manifest::TYPE_WIDGET) {
    const xwalk::application::TizenApplicationInfo* tizen_app_info =
        static_cast<xwalk::application::TizenApplicationInfo*>(
            app_data->GetManifestData(
                xwalk::application_widget_keys::kTizenApplicationKey));
    DCHECK(tizen_app_info);

    strncpy(pkg_detail_info->min_platform_version,
            tizen_app_info->required_version().c_str(),
            PKG_VERSION_STRING_LEN_MAX -1);
  }

  pkg_detail_info->installed_time =
      xwalk::application::GetApplicationInstallationTime(app_data->ID())
          .ToTimeT();  // to seconds

  int install_size = CountAppTotalSize(app_data);
  int data_size = CountAppDataSize(app_data);
  pkg_detail_info->installed_size = install_size;
  pkg_detail_info->app_size = install_size - data_size;
  pkg_detail_info->data_size = data_size;

  strncpy(pkg_detail_info->optional_id, app_data->GetPackageID().c_str(),
          PKG_NAME_STRING_LEN_MAX - 1);
  pkg_detail_info->pkg_optional_info = NULL;
}

scoped_refptr<xwalk::application::ApplicationData>
PkgmgrBackendPlugin::GetApplicationDataFromPkg(const std::string& pkg_path,
                                               base::ScopedTempDir* dir) {
  base::FilePath unpacked_dir = dir->path();
  scoped_ptr<xwalk::application::Package> package =
      xwalk::application::Package::Create(base::FilePath(pkg_path));
  if (!package)
    return nullptr;

  package->ExtractToTemporaryDir(&unpacked_dir);
  std::string error;
  std::string app_id = package->Id();
  scoped_refptr<xwalk::application::ApplicationData> app_data = LoadApplication(
      unpacked_dir, app_id, xwalk::application::ApplicationData::TEMP_DIRECTORY,
      package->manifest_type(), &error);

  return app_data;
}
