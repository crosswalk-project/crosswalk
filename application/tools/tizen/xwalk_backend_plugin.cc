// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/tools/tizen/xwalk_backend_plugin.h"

#include <cstdlib>
#include <cstring>
#include <vector>

#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/files/scoped_temp_dir.h"
#include "base/path_service.h"
#include "base/version.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/application_file_util.h"
#include "xwalk/application/common/id_util.h"
#include "xwalk/runtime/common/xwalk_paths.h"

namespace {

enum PkgmgrPluginBool {
  kPkgmgrPluginTrue = 0,
  kPkgmgrPluginFalse = -1
};

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
  if (!app_data)
    return kPkgmgrPluginFalse;

  SaveDetailInfo(app_data.get(), pkg_detail_info);
  return kPkgmgrPluginTrue;
}

int PkgmgrBackendPlugin::DetailedInfoPkg(
    const std::string& pkg_path,
    package_manager_pkg_detail_info_t* pkg_detail_info) {
  if (!base::PathExists(base::FilePath(pkg_path))) {
    return kPkgmgrPluginFalse;
  }

  scoped_refptr<xwalk::application::ApplicationData> app_data =
      GetApplicationDataFromPkg(pkg_path);
  if (app_data.get() == NULL) {
    return kPkgmgrPluginFalse;
  }

  SaveDetailInfo(app_data.get(), pkg_detail_info);
  return kPkgmgrPluginTrue;
}

int PkgmgrBackendPlugin::IsAppInstalled(const std::string& pkgid) {
  // this will fetch app_id if exists
  std::string app_id = xwalk::application::PkgIdToAppId(pkgid);
  return app_id.empty() ? kPkgmgrPluginFalse : kPkgmgrPluginTrue;
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
      SaveInfo(app_data.get(), result);
      if (*list) {
        result->next = *list;
      }
      *list = result;
      ++*count;
    }
  }
  return kPkgmgrPluginTrue;
}

PkgmgrBackendPlugin::PkgmgrBackendPlugin() {
  base::FilePath data_path;
  xwalk::RegisterPathProvider();
  PathService::Get(xwalk::DIR_DATA_PATH, &data_path);
  storage_.reset(new xwalk::application::ApplicationStorage(data_path));
}

void PkgmgrBackendPlugin::SaveInfo(
    xwalk::application::ApplicationData* app_data,
    package_manager_pkg_info_t* pkg_detail_info) {
  strncpy(pkg_detail_info->pkg_type, "xpk", PKG_TYPE_STRING_LEN_MAX - 1);
  strncpy(pkg_detail_info->pkg_name, app_data->GetPackageID().c_str(),
          PKG_NAME_STRING_LEN_MAX - 1);
  if (app_data->Version() != NULL) {
    strncpy(pkg_detail_info->version, app_data->Version()->GetString().c_str(),
            PKG_VERSION_STRING_LEN_MAX - 1);
  }
}

void PkgmgrBackendPlugin::SaveDetailInfo(
    xwalk::application::ApplicationData* app_data,
    package_manager_pkg_detail_info_t* pkg_detail_info) {
  strncpy(pkg_detail_info->pkg_type, "xpk", PKG_TYPE_STRING_LEN_MAX - 1);
  strncpy(pkg_detail_info->pkg_name, app_data->GetPackageID().c_str(),
          PKG_NAME_STRING_LEN_MAX - 1);
  if (app_data->Version() != NULL) {
    strncpy(pkg_detail_info->version, app_data->Version()->GetString().c_str(),
            PKG_VERSION_STRING_LEN_MAX - 1);
  }
  strncpy(pkg_detail_info->pkg_description, app_data->Description().c_str(),
          PKG_VALUE_STRING_LEN_MAX - 1);

  // TODO(t.iwanek) support this data in ApplicationStorage
  // strncpy(pkg_detail_info.min_platform_version,
  //         app_data->todo, PKG_VERSION_STRING_LEN_MAX -1);
  // PKG_VERSION_STRING_LEN_MAX - 1);
  // pkg_detail_info->installed_time = 0;
  // pkg_detail_info->installed_size = -1;
  // pkg_detail_info->app_size = -1;
  // pkg_detail_info->data_size = -1;

  strncpy(pkg_detail_info->optional_id, app_data->GetPackageID().c_str(),
          PKG_NAME_STRING_LEN_MAX - 1);
  pkg_detail_info->pkg_optional_info = NULL;
}

scoped_refptr<xwalk::application::ApplicationData>
PkgmgrBackendPlugin::GetApplicationDataFromPkg(const std::string& pkg_path) {
  base::ScopedTempDir dir;
  dir.CreateUniqueTempDir();
  base::FilePath unpacked_dir = dir.path();

  scoped_ptr<xwalk::application::Package> package =
      xwalk::application::Package::Create(base::FilePath(pkg_path));
  package->ExtractToTemporaryDir(&unpacked_dir);
  std::string app_id = package->Id();

  std::string error;
  scoped_refptr<xwalk::application::ApplicationData> app_data = LoadApplication(
      unpacked_dir, app_id, xwalk::application::ApplicationData::TEMP_DIRECTORY,
      package->type(), &error);
  return app_data;
}
