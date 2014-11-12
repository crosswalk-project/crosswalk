// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_TOOLS_TIZEN_XWALK_BACKEND_PLUGIN_H_
#define XWALK_APPLICATION_TOOLS_TIZEN_XWALK_BACKEND_PLUGIN_H_

#include <package-manager.h>
#include <package-manager-plugin.h>
#include <package-manager-types.h>

#include <string>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/singleton.h"
#include "xwalk/application/common/tizen/application_storage.h"

template <typename Type>
struct PkgmgrBackendPluginTraits : DefaultSingletonTraits<Type> {
  static const bool kRegisterAtExit = false;
};

class PkgmgrBackendPlugin {
 public:
  static PkgmgrBackendPlugin* GetInstance();

  int DetailedInfo(const std::string& pkgid,
                   package_manager_pkg_detail_info_t* pkg_detail_info);
  int DetailedInfoPkg(const std::string& pkg_path,
                      package_manager_pkg_detail_info_t* pkg_detail_info);
  int IsAppInstalled(const std::string& pkgid);
  int AppsList(package_manager_pkg_info_t** list, int* count);

  void SetLoadSet(pkg_plugin_set* set);

  std::string type() const;

 private:
  PkgmgrBackendPlugin();

  void SaveInfo(scoped_refptr<xwalk::application::ApplicationData> app_data,
                package_manager_pkg_info_t* pkg_detail_info,
                const std::string& force_type = std::string());
  void SaveDetailInfo(
      scoped_refptr<xwalk::application::ApplicationData> app_data,
      package_manager_pkg_detail_info_t* pkg_detail_info,
      const std::string& force_type = std::string());
  scoped_refptr<xwalk::application::ApplicationData> GetApplicationDataFromPkg(
      const std::string& pkg_path, base::ScopedTempDir* dir);

  friend struct DefaultSingletonTraits<PkgmgrBackendPlugin>;

  scoped_ptr<xwalk::application::ApplicationStorage> storage_;
  pkg_plugin_set* set_;

  DISALLOW_COPY_AND_ASSIGN(PkgmgrBackendPlugin);
};

#endif  // XWALK_APPLICATION_TOOLS_TIZEN_XWALK_BACKEND_PLUGIN_H_
