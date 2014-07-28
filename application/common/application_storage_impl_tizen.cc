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

ail_cb_ret_e appinfo_get_exec_cb(const ail_appinfo_h appinfo, void *user_data) {
  char* package_exec;
  ail_appinfo_get_str(appinfo, AIL_PROP_X_SLP_EXE_PATH, &package_exec);
  if (!package_exec)
    return AIL_CB_RET_CONTINUE;

  std::string* x_slp_exe_path = static_cast<std::string*>(user_data);
  *x_slp_exe_path = package_exec;
  return AIL_CB_RET_CANCEL;
}

base::FilePath GetApplicationPath(const std::string& app_id) {
  std::string ail_id = RawAppIdToAppIdForTizenPkgmgrDB(app_id);
  ail_filter_h filter;
  ail_error_e ret = ail_filter_new(&filter);
  if (ret != AIL_ERROR_OK) {
    LOG(ERROR) << "Failed to create AIL filter.";
    return base::FilePath();
  }

  ret = ail_filter_add_str(filter, AIL_PROP_X_SLP_APPID_STR, ail_id.c_str());
  if (ret != AIL_ERROR_OK) {
    LOG(ERROR) << "Failed to init AIL filter.";
    ail_filter_destroy(filter);
    return base::FilePath();
  }

  int count;
  ret = ail_filter_count_appinfo(filter, &count);
  if (ret != AIL_ERROR_OK) {
    LOG(ERROR) << "Failed to count AIL app info.";
    ail_filter_destroy(filter);
    return base::FilePath();
  }

  if (count != 1) {
      LOG(ERROR) << "Invalid count (" << count
                 << ") of the AIL DB records for the app id " << app_id;
    ail_filter_destroy(filter);
    return base::FilePath();
  }

  std::string x_slp_exe_path;
  ail_filter_list_appinfo_foreach(filter, appinfo_get_exec_cb, &x_slp_exe_path);
  ail_filter_destroy(filter);

  // x_slp_exe_path is <app_path>/bin/<app_id>, we need to
  // return just <app_path>.
  std::string toBeExcluded = "/bin/" + app_id;
  unsigned found = x_slp_exe_path.rfind(toBeExcluded);
  if (found == std::string::npos) {
    LOG(ERROR) << "Invalid 'x_slp_exe_path' value (" << x_slp_exe_path
               << ") for the app id " << app_id;
    return base::FilePath();
  }

  CHECK(found < x_slp_exe_path.length());
  x_slp_exe_path.resize(found);
  return base::FilePath(x_slp_exe_path);
}

}  // namespace

scoped_refptr<ApplicationData> ApplicationStorageImpl::GetApplicationData(
    const std::string& app_id) {
  base::FilePath app_path = GetApplicationPath(app_id);

  std::string error_str;
  return LoadApplication(app_path, RawAppIdToCrosswalkAppId(app_id),
                         Manifest::INTERNAL, &error_str);
}

namespace {

int pkgmgrinfo_app_list_cb(pkgmgrinfo_appinfo_h handle, void *user_data) {
  std::vector<std::string>* app_ids =
    static_cast<std::vector<std::string>*>(user_data);
  char* appid = NULL;
  pkgmgrinfo_appinfo_get_appid(handle, &appid);
  CHECK(appid);

  app_ids->push_back(TizenPkgmgrDBAppIdToRawAppId(appid));
  return 0;
}

}  // namespace

bool ApplicationStorageImpl::GetInstalledApplicationIDs(
    std::vector<std::string>& app_ids) {  // NOLINT
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
      handle, pkgmgrinfo_app_list_cb, &app_ids);
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
