// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ail.h>
#include <pkgmgr-info.h>

#include <cstdlib>

#include "base/logging.h"
#include "xwalk/application/common/tizen/package_query.h"

namespace {

typedef ail_cb_ret_e (*PropertyCallback)(const ail_appinfo_h, void*);

ail_cb_ret_e callback_x_slp_exe_path(const ail_appinfo_h appinfo,
    void* user_data) {
  char* package_exec;
  ail_appinfo_get_str(appinfo, AIL_PROP_X_SLP_EXE_PATH, &package_exec);
  if (!package_exec)
    return AIL_CB_RET_CONTINUE;

  std::string* x_slp_exe_path = static_cast<std::string*>(user_data);
  *x_slp_exe_path = package_exec;
  return AIL_CB_RET_CANCEL;
}

ail_cb_ret_e callback_installed_time(const ail_appinfo_h appinfo,
    void* user_data) {
  int* installed_time = static_cast<int*>(user_data);
  ail_appinfo_get_int(appinfo, AIL_PROP_X_SLP_INSTALLEDTIME_INT,
      installed_time);
  return AIL_CB_RET_CANCEL;
}

void GetProperty(const std::string& id,
    const char* type,
    PropertyCallback callback,
    void* user_data) {
  ail_filter_h filter;
  ail_error_e ret = ail_filter_new(&filter);
  if (ret != AIL_ERROR_OK) {
    LOG(ERROR) << "Failed to create AIL filter.";
    return;
  }

  ret = ail_filter_add_str(filter, type, id.c_str());
  if (ret != AIL_ERROR_OK) {
    LOG(ERROR) << "Failed to init AIL filter.";
    ail_filter_destroy(filter);
    return;
  }

  int count;
  uid_t uid = getuid();
  if (uid != GLOBAL_USER)
    ret = ail_filter_count_usr_appinfo(filter, &count, uid);
  else
    ret = ail_filter_count_appinfo(filter, &count);
  if (ret != AIL_ERROR_OK) {
    LOG(ERROR) << "Failed to count AIL app info.";
    ail_filter_destroy(filter);
    return;
  }

  if (count != 1) {
    LOG(ERROR) << "Invalid count (" << count
               << ") of the AIL DB records for the app id " << id;
    ail_filter_destroy(filter);
    return;
  }


  if (uid != GLOBAL_USER)
    ail_filter_list_usr_appinfo_foreach(filter, callback,
                                        user_data, uid);
  else
    ail_filter_list_appinfo_foreach(filter,
                                    callback, user_data);
  ail_filter_destroy(filter);
}

base::FilePath GetPath(const std::string& app_id, const char* type) {
  std::string x_slp_exe_path;
  GetProperty(app_id,
      type,
      callback_x_slp_exe_path,
      static_cast<void*>(&x_slp_exe_path));

  if (x_slp_exe_path.empty()) {
    return base::FilePath();
  }

  // x_slp_exe_path is <app_path>/bin/<app_id>, we need to
  // return just <app_path>.
  return base::FilePath(x_slp_exe_path).DirName().DirName();
}

}  // namespace

namespace xwalk {
namespace application {

base::FilePath GetApplicationPath(const std::string& app_id) {
  return GetPath(app_id, AIL_PROP_X_SLP_APPID_STR);
}

base::FilePath GetPackagePath(const std::string& pkg_id) {
  return GetPath(pkg_id, AIL_PROP_X_SLP_PKGID_STR);
}

base::Time GetApplicationInstallationTime(const std::string& app_id) {
  int installed_time = 0;  // seconds since epoch
  GetProperty(app_id,
      AIL_PROP_X_SLP_APPID_STR,
      callback_installed_time,
      static_cast<void*>(&installed_time));
  return base::Time::FromTimeT(installed_time);
}

}  // namespace application
}  // namespace xwalk
