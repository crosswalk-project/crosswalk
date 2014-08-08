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
  ail_filter_h filter;
  ail_error_e ret = ail_filter_new(&filter);
  if (ret != AIL_ERROR_OK) {
    LOG(ERROR) << "Failed to create AIL filter.";
    return base::FilePath();
  }

  ret = ail_filter_add_str(filter, AIL_PROP_X_SLP_APPID_STR, app_id.c_str());
  if (ret != AIL_ERROR_OK) {
    LOG(ERROR) << "Failed to init AIL filter.";
    ail_filter_destroy(filter);
    return base::FilePath();
  }

  int count;
  ret = ail_filter_count_usr_appinfo(filter, &count, getuid());
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
  ail_filter_list_usr_appinfo_foreach(filter, appinfo_get_exec_cb, &x_slp_exe_path, getuid());
  ail_filter_destroy(filter);

  // x_slp_exe_path is <app_path>/bin/<app_id>, we need to
  // return just <app_path>.
  std::string toBeExcluded = "/bin/" + app_id;
  size_t found = x_slp_exe_path.find(toBeExcluded);
  if (found == std::string::npos) {
    LOG(ERROR) << "Invalid 'x_slp_exe_path' value (" << x_slp_exe_path
               << ") for the app id " << app_id;
    return base::FilePath();
  }

  x_slp_exe_path.resize(found);
  return base::FilePath(x_slp_exe_path);
}

}  // namespace

scoped_refptr<ApplicationData> ApplicationStorageImpl::GetApplicationData(
    const std::string& app_id) {
  base::FilePath app_path = GetApplicationPath(app_id);

  std::string error_str;
  return LoadApplication(app_path, app_id, Manifest::INTERNAL, &error_str);
}

namespace {

ail_cb_ret_e appinfo_get_app_id_cb(
    const ail_appinfo_h appinfo, void* user_data) {
  std::vector<std::string>* app_ids =
    static_cast<std::vector<std::string>*>(user_data);
  char* app_id;
  ail_appinfo_get_str(appinfo, AIL_PROP_X_SLP_APPID_STR, &app_id);
  if (app_id)
    app_ids->push_back(app_id);

  return AIL_CB_RET_CONTINUE;
}

const char kXWalkPackageType[] = "wgt";

}  // namespace

bool ApplicationStorageImpl::GetInstalledApplicationIDs(
  std::vector<std::string>& app_ids) {  // NOLINT
  ail_filter_h filter;
  int count;
  uid_t uid = getuid();

  ail_error_e ret = ail_filter_new(&filter);
  if (ret != AIL_ERROR_OK) {
    LOG(ERROR) << "Failed to create AIL filter.";
    return false;
  }
  // Filters out web apps (installed from WGT and XPK packages).
  ret = ail_filter_add_str(
      filter, AIL_PROP_X_SLP_PACKAGETYPE_STR, kXWalkPackageType);
  if (ret != AIL_ERROR_OK) {
    LOG(ERROR) << "Failed to init AIL filter.";
    ail_filter_destroy(filter);
    return false;
  }

  if(uid != GLOBAL_USER)
    ret = ail_filter_count_usr_appinfo(filter, &count, uid);
  else
    ret = ail_filter_count_appinfo(filter, &count);
  
  if (ret != AIL_ERROR_OK) {
    LOG(ERROR) << "Failed to count AIL app info.";
    ail_filter_destroy(filter);
    return false;
  }

  if (count > 0) {
  	if(uid != GLOBAL_USER)
      ail_filter_list_usr_appinfo_foreach(filter, appinfo_get_app_id_cb,
         &app_ids,uid);
	else
      ail_filter_list_appinfo_foreach(filter, appinfo_get_app_id_cb, &app_ids);
  }
	
  ail_filter_destroy(filter);
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
