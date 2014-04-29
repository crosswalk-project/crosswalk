// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/tools/tizen/xwalk_pkg_installer.h"

#include <assert.h>
#include <stdio.h>
#if defined(OS_TIZEN)
#include <pkgmgr/pkgmgr_parser.h>
#endif

#undef LOG
#include <string>
#include "base/files/file_path.h"
#include "base/file_util.h"

namespace {

typedef int (*PkgParser)(const char*, char* const*);

const base::FilePath kIconDir("/opt/share/icons/default/small/");
const base::FilePath kXmlDir("/opt/share/packages/");
const base::FilePath kXWalkLauncherBinary("/usr/bin/xwalk-launcher");
const std::string kServicePrefix("xwalk-service.");

// Package type sent in every signal.
const char PKGMGR_PKG_TYPE[] = "rpm";

// Notification about operation start.
const char PKGMGR_START_KEY[] = "start";

// Value for new installation.
const char PKGMGR_START_INSTALL[] = "install";

// Value for uninstallation.
const char PKGMGR_START_UNINSTALL[] = "uninstall";

// Notification about end of installation with status.
const char PKGMGR_END_KEY[] = "end";

// Success value of end of installation.
const char PKGMGR_END_SUCCESS[] = "ok";

// Failure value of end of installation.
const char PKGMGR_END_FAILURE[] = "fail";

const std::string kAppIdPrefix("xwalk.");

class FileDeleter {
 public:
  FileDeleter(const base::FilePath& path, bool recursive)
      : path_(path),
        recursive_(recursive) {}

  ~FileDeleter() {
    if (path_.empty())
      return;

    base::DeleteFile(path_, recursive_);
  }

  void Dismiss() {
    path_.clear();
  }

 private:
  base::FilePath path_;
  bool recursive_;
};

}  // namespace

PkgInstaller::PkgInstaller(const std::string& appid)
    : appid_(appid) {
  if (appid_.empty())
    fprintf(stdout, "Invalid app id is provided for pkg installer.\n");

#if defined(OS_TIZEN)
  handle_ = pkgmgr_installer_new();
  if (!handle_)
    fprintf(stdout, "Fail to get package manager installer handle.\n");
#endif
}

PkgInstaller::~PkgInstaller() {
#if defined(OS_TIZEN)
  if (handle_)
    pkgmgr_installer_free(handle_);
#endif
}

bool PkgInstaller::InstallApplication(const std::string& xmlpath,
                                      const std::string& iconpath) {
  if (xmlpath.empty() || iconpath.empty()) {
    fprintf(stdout, "Invalid xml path or icon path for installation\n");
  }

  base::FilePath icon_src(iconpath);
  // icon_dst == /opt/share/icons/default/small/xwalk-service.<appid>.png
  // FIXME(vcgomes): Add support for more icon types
  base::FilePath icon_dst = kIconDir.Append(
      kServicePrefix + std::string(appid_) + ".png");
  if (!base::CopyFile(icon_src, icon_dst)) {
    fprintf(stdout, "Couldn't copy application icon to '%s'\n",
            icon_dst.value().c_str());
    return false;
  }

  FileDeleter icon_cleaner(icon_dst, false);

  base::FilePath xml_src(xmlpath);
  base::FilePath xml_dst = kXmlDir.Append(
      kServicePrefix + std::string(appid_) + ".xml");
  if (!base::CopyFile(xml_src, xml_dst)) {
    fprintf(stdout, "Couldn't copy application XML metadata to '%s'\n",
            xml_dst.value().c_str());
    return false;
  }

#if defined(OS_TIZEN)
  if (!ParseManifest(PkgInstaller::INSTALL, xmlpath)) {
    fprintf(stdout, "Couldn't install %s'\n", appid_.c_str());
    return false;
  }
#endif

  FileDeleter xml_cleaner(xml_dst, false);
  icon_cleaner.Dismiss();
  xml_cleaner.Dismiss();

  return true;
}

bool PkgInstaller::UninstallApplication() {
  bool result = true;

  // FIXME(vcgomes): Add support for more icon types
  base::FilePath icon_dst = kIconDir.Append(
      kServicePrefix + appid_ + ".png");
  if (!base::DeleteFile(icon_dst, false)) {
    fprintf(stdout, "Couldn't delete '%s'\n", icon_dst.value().c_str());
    result = false;
  }

  base::FilePath xmlpath(kXmlDir);
  xmlpath = xmlpath.Append(kServicePrefix + std::string(appid_) + ".xml");

#if defined(OS_TIZEN)
  if (!ParseManifest(PkgInstaller::UNINSTALL, xmlpath.MaybeAsASCII())) {
    fprintf(stdout, "Couldn't uninstall %s'\n", appid_.c_str());
    result = false;
  }
#endif

  if (!base::DeleteFile(xmlpath, false)) {
    fprintf(stdout, "Couldn't delete '%s'\n", xmlpath.value().c_str());
    result = false;
  }

  return result;
}

#if defined(OS_TIZEN)
bool PkgInstaller::ParseManifest(PkgInstaller::RequestType type,
                                 const std::string& xmlpath) {
  std::string pkgmgr_cmd;
  PkgParser parser;
  switch (type) {
    case PkgInstaller::INSTALL:
      pkgmgr_cmd = PKGMGR_START_INSTALL;
      parser = pkgmgr_parser_parse_manifest_for_installation;
      break;

    case PkgInstaller::UNINSTALL:
      pkgmgr_cmd = PKGMGR_START_UNINSTALL;
      parser = pkgmgr_parser_parse_manifest_for_uninstallation;
      break;

    default:
      assert(false);
      fprintf(stdout, "Setting unknown command for package manager.");
      return false;
  }

  SendSignal(PKGMGR_START_KEY, pkgmgr_cmd);

  bool result = parser(xmlpath.c_str(), NULL);

  if (result) {
    fprintf(stdout, "Couldn't parse manifest XML '%s'\n", xmlpath.c_str());
    SendSignal(PKGMGR_END_KEY, PKGMGR_END_FAILURE);
    return false;
  } else {
    SendSignal(PKGMGR_END_KEY, PKGMGR_END_SUCCESS);
  }

  return true;
}

bool PkgInstaller::SendSignal(
    const std::string& key,
    const std::string& value) {
  if (!handle_) {
    fprintf(stdout, "The package install manager is not initialized.\n");
    return false;
  }

  if (key.empty() || value.empty()) {
    fprintf(stdout, " Fail to send signal, key/value is empty.\n");
    return false;
  }

  if (pkgmgr_installer_send_signal(
          handle_, PKGMGR_PKG_TYPE, appid_.c_str(),
          key.c_str(), value.c_str())) {
    fprintf(stdout, "Fail to send package manager signal.\n");
  }

  return true;
}
#endif
