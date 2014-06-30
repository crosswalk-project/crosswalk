// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_INSTALLER_PACKAGE_INSTALLER_H_
#define XWALK_APPLICATION_COMMON_INSTALLER_PACKAGE_INSTALLER_H_

#include <string>
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"

namespace xwalk {
namespace application {

class ApplicationData;
class ApplicationStorage;

class PackageInstaller {
 public:
  static scoped_ptr<PackageInstaller> Create(ApplicationStorage* storage);

  virtual ~PackageInstaller();

  bool Install(const base::FilePath& path, std::string* id);
  bool Uninstall(const std::string& id);
  bool Update(const std::string& id, const base::FilePath& path);

 protected:
  explicit PackageInstaller(ApplicationStorage* storage);
  // Those to be overriden to implement platform specific logic.
  virtual bool PlatformInstall(ApplicationData* data);
  virtual bool PlatformUninstall(ApplicationData* data);
  virtual bool PlatformUpdate(ApplicationData* updated_data);

  ApplicationStorage* storage_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_INSTALLER_PACKAGE_INSTALLER_H_
