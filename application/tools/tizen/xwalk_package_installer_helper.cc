// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/tools/tizen/xwalk_package_installer_helper.h"

#include <assert.h>
#include <stdio.h>
#include <pkgmgr/pkgmgr_parser.h>
#include <tzplatform_config.h>

#undef LOG
#include <string>
#include "base/files/file_path.h"
#include "base/file_util.h"
#include "xwalk/application/common/id_util.h"

namespace {

typedef int (*PkgParser)(const char*, char* const*);

const base::FilePath kXWalkLauncherBinary("/usr/bin/xwalk-launcher");
const char kIconDir[] = "/default/small/";
const std::string kServicePrefix("xwalk-service.");
const std::string kXmlFileExt(".xml");
const std::string kPngFileExt(".png");

// Package type sent in every signal.
const char PKGMGR_PKG_TYPE[] = "wgt";

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

// Tags for pkgmgr describing package
const char* pkgmgr_tags[] = {"removable=true", NULL, };

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
  base::FilePath dir = file_dst.DirName();
  if (!base::PathExists(dir))
    base::CreateDirectory(dir);
  if (!base::CopyFile(file_src, file_dst)) {
    fprintf(stdout, "Couldn't copy application file from %s to '%s'\n",
         file_src.value().c_str(), file_dst.value().c_str());
    return false;
  }
  return true;
}

}  // namespace

PackageInstallerHelper::PackageInstallerHelper(const std::string& appid)
    : handle_(NULL),
      appid_(appid),
      pkgid_(xwalk::application::AppIdToPkgId(appid)) {
  if (appid_.empty())
    fprintf(stdout, "Invalid app id is provided for pkg installer.\n");
}

PackageInstallerHelper::~PackageInstallerHelper() {
  if (handle_)
    pkgmgr_installer_free(handle_);
}

bool PackageInstallerHelper::InitializePkgmgrSignal(int argc,
                                                    const char** argv) {
  handle_ = pkgmgr_installer_new();
  if (!handle_) {
    fprintf(stdout, "Fail to get package manager installer handle.\n");
    return false;
  }

  return pkgmgr_installer_receive_request(handle_,
                                          argc,
                                          const_cast<char**>(argv));
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
  base::FilePath global_xml(tzplatform_mkpath(TZ_SYS_RO_PACKAGES, "/"));
  base::FilePath global_icon(tzplatform_mkpath(TZ_SYS_RO_ICONS, kIconDir));
  base::FilePath user_xml(tzplatform_mkpath(TZ_USER_PACKAGES, "/"));
  base::FilePath user_icon(tzplatform_mkpath(TZ_USER_ICONS, "/"));

  base::FilePath xml(user_xml);
  base::FilePath icon(user_icon);

  uid_t uid = getuid();
  if (uid == GLOBAL_USER) {
    xml = global_xml;
    icon = global_icon;
  }
  // FIXME(vcgomes): Add support for more icon types
  base::FilePath xml_dst = GetDestFilePath(xml, appid_, kXmlFileExt);
  base::FilePath icon_dst = GetDestFilePath(icon, appid_, kPngFileExt);
  FileDeleter xml_cleaner(xml_dst, false);
  FileDeleter icon_cleaner(icon_dst, false);


  if (!CopyFileToDst(base::FilePath(xmlpath), xml_dst)
     || !CopyFileToDst(base::FilePath(iconpath), icon_dst))
    return false;

  fprintf(stdout, "uid for installation : '%d'\n", uid);
  if (uid != GLOBAL_USER) {  // For only the user that request installation
    if (pkgmgr_parser_parse_usr_manifest_for_installation(xmlpath.c_str(),
        uid, const_cast<char**>(pkgmgr_tags))) {
      fprintf(stdout, "Couldn't parse manifest XML '%s', uid : '%d'\n",
              xmlpath.c_str(), uid);
      return false;
    }
  } else {  // For all users
    if (pkgmgr_parser_parse_manifest_for_installation(xmlpath.c_str(),
                                            const_cast<char**>(pkgmgr_tags))) {
      fprintf(stdout, "Couldn't parse manifest XML '%s', uid : '%d'\n",
           xmlpath.c_str(), uid);
      return false;
    }
  }
  xml_cleaner.Dismiss();
  icon_cleaner.Dismiss();

  return true;
}

bool PackageInstallerHelper::UninstallApplicationInternal() {
  base::FilePath global_xml(tzplatform_mkpath(TZ_SYS_RO_PACKAGES, "/"));
  base::FilePath global_icon(tzplatform_mkpath(TZ_SYS_RO_ICONS, kIconDir));
  base::FilePath user_xml(tzplatform_mkpath(TZ_USER_PACKAGES, "/"));
  base::FilePath user_icon(tzplatform_mkpath(TZ_USER_ICONS, "/"));

  base::FilePath xml(user_xml);
  base::FilePath icon(user_icon);

  uid_t uid = getuid();
  if (uid == GLOBAL_USER) {
    xml = global_xml;
    icon = global_icon;
  }
  // FIXME(vcgomes): Add support for more icon types
  base::FilePath iconpath = GetDestFilePath(icon, appid_, kPngFileExt);
  base::FilePath xmlpath = GetDestFilePath(xml, appid_, kXmlFileExt);
  FileDeleter icon_cleaner(iconpath, false);
  FileDeleter xml_cleaner(xmlpath, false);

  std::string xmlpath_str = xmlpath.MaybeAsASCII();
  assert(!xmlpath_str.empty());

  if (uid != GLOBAL_USER) {  // For only the user that request installation
    if (pkgmgr_parser_parse_usr_manifest_for_uninstallation(
        xmlpath_str.c_str(), uid, NULL)) {
      fprintf(stdout, "Couldn't parse manifest XML '%s'\n",
              xmlpath_str.c_str());
      icon_cleaner.Dismiss();
      xml_cleaner.Dismiss();
    }
  } else {  // For all users
    if (pkgmgr_parser_parse_manifest_for_uninstallation(
        xmlpath_str.c_str(), NULL)) {
      fprintf(stdout, "Couldn't parse manifest XML '%s'\n",
              xmlpath_str.c_str());
      icon_cleaner.Dismiss();
      xml_cleaner.Dismiss();
    }
  }
  return true;
}

bool PackageInstallerHelper::UpdateApplicationInternal(
    const std::string& xmlpath,
    const std::string& iconpath) {
  if (xmlpath.empty() || iconpath.empty()) {
    fprintf(stdout, "Invalid xml path or icon path for update\n");
  }

  base::FilePath global_xml(tzplatform_mkpath(TZ_SYS_RO_PACKAGES, "/"));
  base::FilePath global_icon(tzplatform_mkpath(TZ_SYS_RO_ICONS, kIconDir));
  base::FilePath user_xml(tzplatform_mkpath(TZ_USER_PACKAGES, "/"));
  base::FilePath user_icon(tzplatform_mkpath(TZ_USER_ICONS, "/"));

  base::FilePath xml(user_xml);
  base::FilePath icon(user_icon);

  uid_t uid = getuid();
  if (uid == GLOBAL_USER) {
    xml = global_xml;
    icon = global_icon;
  }

  // FIXME(vcgomes): Add support for more icon types
  base::FilePath xml_dst = GetDestFilePath(xml, appid_, kXmlFileExt);
  base::FilePath icon_dst = GetDestFilePath(icon, appid_, kPngFileExt);
  FileDeleter xml_cleaner(xml_dst, false);
  FileDeleter icon_cleaner(icon_dst, false);


  if (!CopyFileToDst(base::FilePath(xmlpath), xml_dst)
     || !CopyFileToDst(base::FilePath(iconpath), icon_dst))
    return false;

  if (uid != GLOBAL_USER) {  // For only the user that request installation
    if (pkgmgr_parser_parse_usr_manifest_for_upgrade(xmlpath.c_str(), uid,
        const_cast<char**>(pkgmgr_tags))) {
      fprintf(stdout, "Couldn't parse manifest XML '%s'\n", xmlpath.c_str());
      return false;
    }
  } else {  // For all users
    if (pkgmgr_parser_parse_manifest_for_upgrade(xmlpath.c_str(),
                                            const_cast<char**>(pkgmgr_tags))) {
      fprintf(stdout, "Couldn't parse manifest XML '%s'\n", xmlpath.c_str());
      return false;
     }
  }

  xml_cleaner.Dismiss();
  icon_cleaner.Dismiss();

  return true;
}

bool PackageInstallerHelper::SendSignal(
    const std::string& key,
    const std::string& value) {
  if (!handle_) {
    // this is installation with xwalkctl not pkgmgr
    return true;
  }

  if (key.empty() || value.empty()) {
    fprintf(stdout, " Fail to send signal, key/value is empty.\n");
    return false;
  }

  if (pkgmgr_installer_send_signal(
          handle_, PKGMGR_PKG_TYPE, pkgid_.c_str(),
          key.c_str(), value.c_str())) {
    fprintf(stdout, "Fail to send package manager signal.\n");
  }

  return true;
}
