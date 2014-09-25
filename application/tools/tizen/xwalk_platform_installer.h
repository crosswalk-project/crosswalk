// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_TOOLS_TIZEN_XWALK_PLATFORM_INSTALLER_H_
#define XWALK_APPLICATION_TOOLS_TIZEN_XWALK_PLATFORM_INSTALLER_H_

#include <string>

#include "base/files/file_path.h"

struct pkgmgr_installer;

class PlatformInstaller {
 public:
  PlatformInstaller();
  explicit PlatformInstaller(const std::string& appid);
  ~PlatformInstaller();
  bool InitializePkgmgrSignal(int argc, const char** argv);

  bool InstallApplication(const base::FilePath& xmlpath,
                          const base::FilePath& iconpath);
  bool UninstallApplication();
  bool UpdateApplication(const base::FilePath& xmlpath,
                         const base::FilePath& iconpath);
  bool ReinstallApplication();

 private:
  bool InstallApplicationInternal(const base::FilePath& xmlpath,
                                  const base::FilePath& iconpath);
  bool UninstallApplicationInternal();
  bool UpdateApplicationInternal(const base::FilePath& xmlpath,
                                 const base::FilePath& iconpath);

  bool SendSignal(const std::string& key, const std::string& value);

  pkgmgr_installer* handle_;

  std::string appid_;
  std::string pkgid_;
};

#endif  // XWALK_APPLICATION_TOOLS_TIZEN_XWALK_PLATFORM_INSTALLER_H_
