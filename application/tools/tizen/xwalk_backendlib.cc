// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <package-manager.h>
#include <package-manager-types.h>
#include <package-manager-plugin.h>

#include "base/logging.h"
#include "xwalk/application/tools/tizen/xwalk_backend_plugin.h"

#define API_EXPORT __attribute__((visibility("default")))

extern "C" {

// exported function
API_EXPORT int pkg_plugin_on_load(pkg_plugin_set *set);

void pkg_plugin_on_unload();
int pkg_plugin_get_app_detail_info(
    const char* pkg_name, package_manager_pkg_detail_info_t* pkg_detail_info);
int pkg_plugin_get_app_detail_info_from_package(
    const char* pkg_path, package_manager_pkg_detail_info_t* pkg_detail_info);
int pkg_plugin_app_is_installed(const char* pkg_name);
int pkg_plugin_get_installed_apps_list(
    const char* category, const char* option, package_manager_pkg_info_t** list,
    int* count);

// definitions
int pkg_plugin_on_load(pkg_plugin_set* set) {
  LOG(INFO) << "Crosswalk backend plugin - load";

  if (!set) {
    return FALSE;
  }
  memset(set, 0x00, sizeof(pkg_plugin_set));

  set->plugin_on_unload = pkg_plugin_on_unload;
  set->pkg_is_installed = pkg_plugin_app_is_installed;
  set->get_installed_pkg_list = pkg_plugin_get_installed_apps_list;
  set->get_pkg_detail_info = pkg_plugin_get_app_detail_info;
  set->get_pkg_detail_info_from_package =
      pkg_plugin_get_app_detail_info_from_package;

  // FIXME: store load set which contains
  // pkgmgr sets pkg_type after calling 'pkg_plugin_on_load'
  // we need to store load set to recover type of package
  // for which backendlib was loaded - wgt or xpk
  PkgmgrBackendPlugin::GetInstance()->SetLoadSet(set);

  return 0;
}

void pkg_plugin_on_unload() {
  LOG(INFO) << "Crosswalk backend plugin ("
            << PkgmgrBackendPlugin::GetInstance()->type()
            << ") - unload";
}

int pkg_plugin_get_app_detail_info(
    const char *pkg_name, package_manager_pkg_detail_info_t *pkg_detail_info) {
  LOG(INFO) << "Crosswalk backend plugin ("
            << PkgmgrBackendPlugin::GetInstance()->type()
            << ") - pkg_plugin_get_app_detail_info";

  return PkgmgrBackendPlugin::GetInstance()->DetailedInfo(pkg_name,
                                                          pkg_detail_info);
}

int pkg_plugin_get_app_detail_info_from_package(
    const char *pkg_path, package_manager_pkg_detail_info_t *pkg_detail_info) {
  LOG(INFO) << "Crosswalk backend plugin ("
            << PkgmgrBackendPlugin::GetInstance()->type()
            << ") - pkg_plugin_get_app_detail_info_from_package";

  return PkgmgrBackendPlugin::GetInstance()->DetailedInfoPkg(pkg_path,
                                                             pkg_detail_info);
}

int pkg_plugin_app_is_installed(const char *pkg_name) {
  LOG(INFO) << "Crosswalk backend plugin ("
            << PkgmgrBackendPlugin::GetInstance()->type()
            << ") - pkg_plugin_app_is_installed";

  return PkgmgrBackendPlugin::GetInstance()->IsAppInstalled(pkg_name);
}

int pkg_plugin_get_installed_apps_list(const char* category,
                                       const char* option,
                                       package_manager_pkg_info_t** list,
                                       int* count) {
  LOG(INFO) << "Crosswalk backend plugin ("
            << PkgmgrBackendPlugin::GetInstance()->type()
            << ") - pkg_plugin_get_installed_apps_list";

  return PkgmgrBackendPlugin::GetInstance()->AppsList(list, count);
}

}
