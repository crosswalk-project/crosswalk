// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/tools/tizen/xwalk_package_installer_helper.h"

#include <assert.h>
#include <stdio.h>
#include <pkgmgr/pkgmgr_parser.h>

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
const std::string kXmlFileExt(".xml");
const std::string kPngFileExt(".png");

// Package type sent in every signal.
const char PKGMGR_PKG_TYPE[] = "rpm";

// Notification about operation start.
const char PKGMGR_START_KEY[] = "start";

// Value for new installation.
const char PKGMGR_START_INSTALL[] = "install";

// Value for uninstallation.
const char PKGMGR_START_UNINSTALL[] = "uninstall";

// Value for update.
const char PKGMGR_START_UPDATE[] = "update";

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

const char* ToEndStatus(bool result) {
  return result ? PKGMGR_END_SUCCESS : PKGMGR_END_FAILURE;
}

inline base::FilePath GetDestFilePath(const base::FilePath& DirPath,
                                      const std::string& appid,
                                      const std::string& file_ext) {
  return DirPath.Append(kServicePrefix + std::string(appid) + file_ext);
}

bool CopyFileToDst(const base::FilePath& file_src,
                   const base::FilePath& file_dst) {
  if (!base::CopyFile(file_src, file_dst)) {
    fprintf(stdout, "Couldn't copy application file to '%s'\n",
            file_dst.value().c_str());
    return false;
  }
  return true;
}

}  // namespace

PackageInstallerHelper::PackageInstallerHelper(const std::string& appid)
    : appid_(appid) {
  if (appid_.empty())
    fprintf(stdout, "Invalid app id is provided for pkg installer.\n");

  handle_ = pkgmgr_installer_new();
  if (!handle_)
    fprintf(stdout, "Fail to get package manager installer handle.\n");
}

PackageInstallerHelper::~PackageInstallerHelper() {
  if (handle_)
    pkgmgr_installer_free(handle_);
}

bool PackageInstallerHelper::InstallApplication(
    const std::string& xmlpath,
    const std::string& iconpath) {
  SendSignal(PKGMGR_START_KEY, PKGMGR_START_INSTALL);
  bool ret = InstallApplicationInternal(xmlpath, iconpath);
  SendSignal(PKGMGR_END_KEY, ToEndStatus(ret));
  return ret;
}

bool PackageInstallerHelper::UninstallApplication() {
  SendSignal(PKGMGR_START_KEY, PKGMGR_START_UNINSTALL);
  bool ret = UninstallApplicationInternal();
  SendSignal(PKGMGR_END_KEY, ToEndStatus(ret));
  return ret;
}

bool PackageInstallerHelper::UpdateApplication(
    const std::string& xmlpath,
    const std::string& iconpath) {
  SendSignal(PKGMGR_START_KEY, PKGMGR_START_UPDATE);
  bool ret = UpdateApplicationInternal(xmlpath, iconpath);
  SendSignal(PKGMGR_END_KEY, ToEndStatus(ret));
  return ret;
}

bool PackageInstallerHelper::InstallApplicationInternal(
    const std::string& xmlpath,
    const std::string& iconpath) {
  if (xmlpath.empty() || iconpath.empty()) {
    fprintf(stdout, "Invalid xml path or icon path for installation\n");
  }

  // FIXME(vcgomes): Add support for more icon types
  base::FilePath xml_dst = GetDestFilePath(kXmlDir, appid_, kXmlFileExt);
  base::FilePath icon_dst = GetDestFilePath(kIconDir, appid_, kPngFileExt);
  FileDeleter xml_cleaner(xml_dst, false);
  FileDeleter icon_cleaner(icon_dst, false);


  if (!CopyFileToDst(base::FilePath(xmlpath), xml_dst)
     || !CopyFileToDst(base::FilePath(iconpath), icon_dst))
    return false;

  if (pkgmgr_parser_parse_manifest_for_installation(xmlpath.c_str(), NULL)) {
    fprintf(stdout, "Couldn't parse manifest XML '%s'\n", xmlpath.c_str());
    return false;
  }

  xml_cleaner.Dismiss();
  icon_cleaner.Dismiss();

  return true;
}

bool PackageInstallerHelper::UninstallApplicationInternal() {
  bool result = true;

  // FIXME(vcgomes): Add support for more icon types
  base::FilePath iconpath = GetDestFilePath(kIconDir, appid_, kPngFileExt);
  base::FilePath xmlpath = GetDestFilePath(kXmlDir, appid_, kXmlFileExt);
  FileDeleter icon_cleaner(iconpath, false);
  FileDeleter xml_cleaner(xmlpath, false);

  std::string xmlpath_str = xmlpath.MaybeAsASCII();
  assert(!xmlpath_str.empty());
  if (pkgmgr_parser_parse_manifest_for_uninstallation(
        xmlpath_str.c_str(), NULL)) {
    fprintf(stdout, "Couldn't parse manifest XML '%s'\n", xmlpath_str.c_str());
    icon_cleaner.Dismiss();
    xml_cleaner.Dismiss();
    return false;
  }
  return true;
}

bool PackageInstallerHelper::UpdateApplicationInternal(
    const std::string& xmlpath,
    const std::string& iconpath) {
  if (xmlpath.empty() || iconpath.empty()) {
    fprintf(stdout, "Invalid xml path or icon path for update\n");
  }

  // FIXME(vcgomes): Add support for more icon types
  base::FilePath xml_dst = GetDestFilePath(kXmlDir, appid_, kXmlFileExt);
  base::FilePath icon_dst = GetDestFilePath(kIconDir, appid_, kPngFileExt);
  FileDeleter xml_cleaner(xml_dst, false);
  FileDeleter icon_cleaner(icon_dst, false);


  if (!CopyFileToDst(base::FilePath(xmlpath), xml_dst)
     || !CopyFileToDst(base::FilePath(iconpath), icon_dst))
    return false;

  if (pkgmgr_parser_parse_manifest_for_upgrade(xmlpath.c_str(), NULL)) {
    fprintf(stdout, "Couldn't parse manifest XML '%s'\n", xmlpath.c_str());
    return false;
  }

  xml_cleaner.Dismiss();
  icon_cleaner.Dismiss();

  return true;
}

bool PackageInstallerHelper::SendSignal(
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
