// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_TOOLS_TIZEN_XWALK_PACKAGE_INSTALLER_HELPER_H_
#define XWALK_APPLICATION_TOOLS_TIZEN_XWALK_PACKAGE_INSTALLER_HELPER_H_

#include <pkgmgr_installer.h>

#include <string>

class PackageInstallerHelper {
 public:
  explicit PackageInstallerHelper(const std::string& appid);
  ~PackageInstallerHelper();
  bool InstallApplication(const std::string& xmlpath,
                          const std::string& iconpath);
  bool UninstallApplication();
  bool UpdateApplication(const std::string& xmlpath,
                         const std::string& iconpath);

 private:
  bool InstallApplicationInternal(const std::string& xmlpath,
                                  const std::string& iconpath);
  bool UninstallApplicationInternal();
  bool UpdateApplicationInternal(const std::string& xmlpath,
                                 const std::string& iconpath);

  bool SendSignal(const std::string& key, const std::string& value);

  pkgmgr_installer* handle_;

  std::string appid_;
};

#endif  // XWALK_APPLICATION_TOOLS_TIZEN_XWALK_PACKAGE_INSTALLER_HELPER_H_
