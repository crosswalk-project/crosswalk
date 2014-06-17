// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_INSTALLER_PACKAGE_INSTALLER_TIZEN_H_
#define XWALK_APPLICATION_BROWSER_INSTALLER_PACKAGE_INSTALLER_TIZEN_H_

#include <string>

#include "xwalk/application/browser/installer/package_installer.h"

namespace xwalk {
namespace application {

class PackageInstallerTizen : public PackageInstaller {
 public:
  explicit PackageInstallerTizen(ApplicationStorage* storage);

 private:
  virtual bool PlatformInstall(ApplicationData* data) OVERRIDE;
  virtual bool PlatformUninstall(ApplicationData* data) OVERRIDE;
  virtual bool PlatformUpdate(ApplicationData* data) OVERRIDE;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_INSTALLER_PACKAGE_INSTALLER_TIZEN_H_
