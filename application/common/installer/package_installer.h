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
  bool Reinstall(const base::FilePath& path);

  void ContinueUnfinishedTasks();

  virtual void SetQuiet(bool quiet);
  virtual void SetInstallationKey(const std::string& key);

 protected:
  explicit PackageInstaller(ApplicationStorage* storage);

  virtual std::string PrepareUninstallationID(const std::string& id);

  // Those to be overriden to implement platform specific logic.
  virtual bool PlatformInstall(ApplicationData* data);
  virtual bool PlatformUninstall(ApplicationData* data);
  virtual bool PlatformUpdate(ApplicationData* updated_data);

  virtual bool PlatformReinstall(const base::FilePath& path);

  ApplicationStorage* storage_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_INSTALLER_PACKAGE_INSTALLER_H_
