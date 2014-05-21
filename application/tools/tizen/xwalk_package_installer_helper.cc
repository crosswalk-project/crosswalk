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

  if (pkgmgr_parser_parse_manifest_for_installation(xmlpath.c_str(), NULL)) {
    fprintf(stdout, "Couldn't parse manifest XML '%s'\n", xmlpath.c_str());
    return false;
  }

  FileDeleter xml_cleaner(xml_dst, false);
  icon_cleaner.Dismiss();
  xml_cleaner.Dismiss();

  return true;
}

bool PackageInstallerHelper::UninstallApplicationInternal() {
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

  std::string xmlpath_str = xmlpath.MaybeAsASCII();
  assert(!xmlpath_str.empty());
  if (pkgmgr_parser_parse_manifest_for_uninstallation(
        xmlpath_str.c_str(), NULL)) {
    fprintf(stdout, "Couldn't parse manifest XML '%s'\n", xmlpath_str.c_str());
    result = false;
  }

  if (!base::DeleteFile(xmlpath, false)) {
    fprintf(stdout, "Couldn't delete '%s'\n", xmlpath_str.c_str());
    result = false;
  }

  return result;
}

bool PackageInstallerHelper::UpdateApplicationInternal(
    const std::string& xmlpath,
    const std::string& iconpath) {
  if (xmlpath.empty() || iconpath.empty()) {
    fprintf(stdout, "Invalid xml path or icon path for update\n");
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

  if (pkgmgr_parser_parse_manifest_for_upgrade(xmlpath.c_str(), NULL)) {
    fprintf(stdout, "Couldn't parse manifest XML '%s'\n", xmlpath.c_str());
    return false;
  }

  FileDeleter xml_cleaner(xml_dst, false);
  icon_cleaner.Dismiss();
  xml_cleaner.Dismiss();

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
