// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_INSTALLER_TIZEN_PACKAGE_INSTALLER_H_
#define XWALK_APPLICATION_BROWSER_INSTALLER_TIZEN_PACKAGE_INSTALLER_H_

#include <unistd.h>
#include <string>
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "xwalk/application/browser/application_service.h"

namespace xwalk {
namespace application {

class PackageInstaller
    : public base::RefCountedThreadSafe<PackageInstaller> {
 public:
  PackageInstaller();
  ~PackageInstaller();
  static scoped_refptr<PackageInstaller> Create(
      ApplicationService* service,
      const std::string& package_id,
      const base::FilePath& data_dir);
  bool Install();
  bool Uninstall();

 private:
  friend class base::RefCountedThreadSafe<PackageInstaller>;
  explicit PackageInstaller(
      ApplicationService* service,
      const std::string& package_id,
      const base::FilePath& data_dir);
  bool Init();
  bool GeneratePkgInfoXml();
  bool CopyOrLinkResources();
  bool WriteToPackageInfoDB();
  bool ChangeOwnerRecursive(
      const base::FilePath& path,
      const uid_t& uid,
      const gid_t& gid);

  const ApplicationService* service_;
  scoped_refptr<const Application> application_;
  std::string package_id_;
  std::string icon_name_;
  std::string stripped_name_;
  base::FilePath data_dir_;
  base::FilePath app_dir_;
  base::FilePath xml_path_;
  base::FilePath execute_path_;
  base::FilePath icon_path_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_INSTALLER_TIZEN_PACKAGE_INSTALLER_H_
