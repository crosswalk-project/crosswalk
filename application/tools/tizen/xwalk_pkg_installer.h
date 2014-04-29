// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_TOOLS_TIZEN_XWALK_PKG_INSTALLER_H_
#define XWALK_APPLICATION_TOOLS_TIZEN_XWALK_PKG_INSTALLER_H_

#if defined(OS_TIZEN)
#include <pkgmgr_installer.h>
#endif

#include <string>

class PkgInstaller {
 public:
  explicit PkgInstaller(const std::string& appid);
  ~PkgInstaller();
  bool InstallApplication(const std::string& xmlpath,
                          const std::string& iconpath);
  bool UninstallApplication();

 private:
#if defined(OS_TIZEN)
  enum RequestType {
    INSTALL,
    UNINSTALL
  };
  bool ParseManifest(RequestType type, const std::string& xmlpath);
  bool SendSignal(const std::string& key, const std::string& value);

  pkgmgr_installer* handle_;
#endif

  std::string appid_;
};

#endif  // XWALK_APPLICATION_TOOLS_TIZEN_XWALK_PKG_INSTALLER_H_
