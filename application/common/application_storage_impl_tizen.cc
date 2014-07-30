// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/application_storage_impl_tizen.h"

#include <ail.h>
#include <pkgmgr-info.h>

#include <string>
#include <vector>

#include "base/file_util.h"
#include "third_party/re2/re2/re2.h"
#include "xwalk/application/common/application_file_util.h"
#include "xwalk/application/common/application_storage.h"
#include "xwalk/application/common/id_util.h"

namespace xwalk {
namespace application {

ApplicationStorageImpl::ApplicationStorageImpl(const base::FilePath& path) {
}

ApplicationStorageImpl::~ApplicationStorageImpl() {
}

bool ApplicationStorageImpl::Init() {
  return true;
}

namespace {

base::FilePath GetApplicationPath(const std::string& app_id) {
  pkgmgrinfo_appinfo_h handle;
  char* exec_path = NULL;
  if (pkgmgrinfo_appinfo_get_appinfo(app_id.c_str(), &handle) != PMINFO_R_OK ||
      pkgmgrinfo_appinfo_get_exec(handle, &exec_path) != PMINFO_R_OK ||
      !exec_path) {
    LOG(ERROR) << "Couldn't find exec path for application: " << app_id;
    return base::FilePath();
  }

  std::string exe_path(exec_path);

  // exe_path is <app_path>/bin/<app_id>, we need to
  // return just <app_path>.
  std::string toBeExcluded = "/bin/" + app_id;
  size_t found = exe_path.rfind(toBeExcluded);
  if (found == std::string::npos) {
    LOG(ERROR) << "Invalid 'exe_path' value (" << exe_path
               << ") for the app id " << app_id;
    return base::FilePath();
  }

  exe_path.resize(found);
  return base::FilePath(exe_path);
}

}  // namespace

scoped_refptr<ApplicationData> ApplicationStorageImpl::GetApplicationData(
    const std::string& tizen_app_id) {
  base::FilePath app_path = GetApplicationPath(tizen_app_id);

  if (app_path.empty())
    return NULL;
  std::string error_str;
  return LoadApplication(app_path, TizenAppIdToAppId(tizen_app_id),
                         Manifest::INTERNAL, &error_str);
}

namespace {

int pkgmgrinfo_app_list_cb(pkgmgrinfo_appinfo_h handle, void *user_data) {
  std::vector<std::string>* tizen_app_ids =
    static_cast<std::vector<std::string>*>(user_data);
  char* tizen_app_id = NULL;
  pkgmgrinfo_appinfo_get_appid(handle, &tizen_app_id);
  CHECK(tizen_app_id);

  tizen_app_ids->push_back(tizen_app_id);
  return 0;
}

}  // namespace

bool ApplicationStorageImpl::GetInstalledApplicationIDs(
    std::vector<std::string>& tizen_app_ids) {  // NOLINT
  pkgmgrinfo_appinfo_filter_h handle;
  int ret = pkgmgrinfo_appinfo_filter_create(&handle);
  if (ret != PMINFO_R_OK) {
    LOG(ERROR) << "Failed to create pkgmgrinfo filter.";
    return false;
  }

  ret = pkgmgrinfo_appinfo_filter_add_string(
      handle, PMINFO_APPINFO_PROP_APP_TYPE, "webapp");
  if (ret != PMINFO_R_OK) {
    LOG(ERROR) << "Failed to init pkgmgrinfo filter.";
    pkgmgrinfo_appinfo_filter_destroy(handle);
    return false;
  }

  ret = pkgmgrinfo_appinfo_filter_foreach_appinfo(
      handle, pkgmgrinfo_app_list_cb, &tizen_app_ids);
  if (ret != PMINFO_R_OK) {
    LOG(ERROR) << "Failed to apply pkgmgrinfo filter.";
    pkgmgrinfo_appinfo_filter_destroy(handle);
    return false;
  }
  pkgmgrinfo_appinfo_filter_destroy(handle);

  return true;
}

bool ApplicationStorageImpl::AddApplication(const ApplicationData* application,
                                            const base::Time& install_time) {
  return true;
}

bool ApplicationStorageImpl::UpdateApplication(
    ApplicationData* application, const base::Time& install_time) {
  return true;
}

bool ApplicationStorageImpl::RemoveApplication(const std::string& id) {
  return true;
}

bool ApplicationStorageImpl::ContainsApplication(const std::string& app_id) {
  return !GetApplicationPath(app_id).empty();
}

}  // namespace application
}  // namespace xwalk
