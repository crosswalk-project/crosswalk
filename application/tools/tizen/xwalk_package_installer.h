// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_TOOLS_TIZEN_XWALK_PACKAGE_INSTALLER_H_
#define XWALK_APPLICATION_TOOLS_TIZEN_XWALK_PACKAGE_INSTALLER_H_

#include <string>
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"

namespace xwalk {
namespace application {

class ApplicationData;
class ApplicationStorage;

}  // namespace application
}  // namespace xwalk

class PackageInstaller {
 public:
  static scoped_ptr<PackageInstaller> Create(
    xwalk::application::ApplicationStorage* storage);

  ~PackageInstaller();

  bool Install(const base::FilePath& path, std::string* id);
  bool Uninstall(const std::string& id);
  bool Update(const std::string& id, const base::FilePath& path);
  bool Reinstall(const base::FilePath& path);

  void ContinueUnfinishedTasks();

  void SetQuiet(bool quiet);
  void SetInstallationKey(const std::string& key);

 protected:
  explicit PackageInstaller(xwalk::application::ApplicationStorage* storage);

  std::string PrepareUninstallationID(const std::string& id);

  bool PlatformInstall(xwalk::application::ApplicationData* data);
  bool PlatformUninstall(xwalk::application::ApplicationData* data);
  bool PlatformUpdate(xwalk::application::ApplicationData* updated_data);
  bool PlatformReinstall(const base::FilePath& path);

  xwalk::application::ApplicationStorage* storage_;
  bool quiet_;
  std::string key_;
};



#endif  // XWALK_APPLICATION_TOOLS_TIZEN_XWALK_PACKAGE_INSTALLER_H_
