// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_INSTALLER_PACKAGE_INSTALLER_TIZEN_H_
#define XWALK_APPLICATION_COMMON_INSTALLER_PACKAGE_INSTALLER_TIZEN_H_

#include <string>

#include "xwalk/application/common/installer/package_installer.h"

namespace xwalk {
namespace application {

class PackageInstallerTizen : public PackageInstaller {
 public:
  explicit PackageInstallerTizen(ApplicationStorage* storage);

  void SetQuiet(bool quiet) OVERRIDE;
  void SetInstallationKey(const std::string& key) OVERRIDE;

 protected:
  std::string PrepareUninstallationID(const std::string& id) OVERRIDE;

  virtual bool PlatformInstall(ApplicationData* data) OVERRIDE;
  virtual bool PlatformUninstall(ApplicationData* data) OVERRIDE;
  virtual bool PlatformUpdate(ApplicationData* data) OVERRIDE;

 private:
  bool quiet_;
  std::string key_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_INSTALLER_PACKAGE_INSTALLER_TIZEN_H_
