// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/installer/tizen/package_installer.h"

#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <pkgmgr/pkgmgr_parser.h>

#include <algorithm>
#include <string>
#include "base/file_util.h"
#include "base/files/file_enumerator.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "third_party/libxml/chromium/libxml_utils.h"
#include "xwalk/application/browser/application_store.h"
#include "xwalk/application/browser/installer/tizen/packageinfo_constants.h"

namespace info = xwalk::application_packageinfo_constants;

namespace xwalk {
namespace application {

PackageInstaller::~PackageInstaller() {
}

// static
scoped_ptr<PackageInstaller> PackageInstaller::Create(
    ApplicationService* service,
    const std::string& package_id,
    const base::FilePath& data_dir) {
  if (!base::PathExists(data_dir))
    return scoped_ptr<PackageInstaller>();
  scoped_ptr<PackageInstaller> handler(
      new PackageInstaller(service, package_id, data_dir));
  if (!handler->Init())
    return scoped_ptr<PackageInstaller>();
  return handler.Pass();
}

PackageInstaller::PackageInstaller(
    ApplicationService* service,
    const std::string& package_id,
    const base::FilePath& data_dir)
    : service_(service)
    , package_id_(package_id)
    , data_dir_(data_dir) {
  CHECK(service_);
}

bool PackageInstaller::Init() {
  app_dir_ = data_dir_.Append(info::kAppDir).AppendASCII(package_id_);
  xml_path_ = base::FilePath(info::kXmlDir)
      .AppendASCII(package_id_ + std::string(info::kXmlExtension));
  execute_path_ = app_dir_.Append(info::kExecDir).AppendASCII(package_id_);

  application_ = service_->GetApplicationByID(package_id_);
  if (!application_) {
    LOG(ERROR) << "Application " << package_id_
               << " haven't been installed in Xwalk database.";
    return false;
  }
  stripped_name_ = application_->Name();
  stripped_name_.erase(
      std::remove_if(stripped_name_.begin(), stripped_name_.end(), ::isspace),
      stripped_name_.end());

  if (!application_->GetManifest()->GetString(info::kIconKey, &icon_name_))
    LOG(WARNING) << "Fail to get application icon name.";

  icon_path_ = base::FilePath(info::kIconDir).AppendASCII(
      package_id_ + info::kSeparator + stripped_name_ +
      base::FilePath::FromUTF8Unsafe(icon_name_).Extension());
  return true;
}

bool PackageInstaller::GeneratePkgInfoXml() {
  base::FilePath dir_xml(xml_path_.DirName());
  if (!base::PathExists(dir_xml) &&
      !file_util::CreateDirectory(dir_xml))
    return false;

  FILE* file = file_util::OpenFile(xml_path_, "w");
  XmlWriter xml_writer;
  xml_writer.StartWriting();
  xml_writer.StartElement("manifest");
  xml_writer.AddAttribute("xmlns", "http://tizen.org/ns/packages");
  xml_writer.AddAttribute("package", package_id_);
  xml_writer.AddAttribute("version", application_->VersionString());
  xml_writer.WriteElement("label", application_->Name());
  xml_writer.WriteElement("description", application_->Description());

  {
    xml_writer.StartElement("ui-application");
    xml_writer.AddAttribute("appid",
        package_id_ + info::kSeparator + stripped_name_);
    xml_writer.AddAttribute("exec", execute_path_.MaybeAsASCII());
    xml_writer.AddAttribute("type", "c++app");
    xml_writer.AddAttribute("taskmanage", "true");
    xml_writer.WriteElement("label", application_->Name());
    if (icon_name_.empty())
      xml_writer.WriteElement("icon", info::kDefaultIconName);
    else
      xml_writer.WriteElement("icon", icon_path_.BaseName().MaybeAsASCII());
    xml_writer.EndElement();  // Ends "ui-application"
  }

  xml_writer.EndElement();  // Ends "manifest" element.
  xml_writer.StopWriting();

  file_util::WriteFile(xml_path_,
                       xml_writer.GetWrittenString().c_str(),
                       xml_writer.GetWrittenString().size());
  file_util::CloseFile(file);
  LOG(INFO) << "Converting manifest.json into "
            << xml_path_.BaseName().MaybeAsASCII()
            << " for installation. [DONE]";
  return true;
}

bool PackageInstaller::CopyOrLinkResources() {
  base::FilePath icon = app_dir_.AppendASCII(icon_name_);
  if (!icon_name_.empty() && base::PathExists(icon))
    base::CopyFile(icon, icon_path_);

  base::FilePath xwalk_path(info::kXwalkPath);
  base::FilePath dir_exec(execute_path_.DirName());
  if (!base::PathExists(dir_exec))
    file_util::CreateDirectory(dir_exec);

  file_util::CreateSymbolicLink(xwalk_path, execute_path_);
  LOG(INFO) << "Copying and linking files into correct locations. [DONE]";
  return true;
}

bool PackageInstaller::WriteToPackageInfoDB() {
  uid_t uid;
  gid_t gid;
  struct passwd pwd;
  char pwd_buffer[1024];
  struct passwd *res;

  getpwnam_r(info::kOwner, &pwd, pwd_buffer, sizeof(pwd_buffer), &res);
  if (res == NULL) {
    LOG(ERROR) << "Fail to get PW name";
    return false;
  }
  uid = pwd.pw_uid;
  gid = pwd.pw_gid;
  if (!ChangeOwnerRecursive(data_dir_.Append(info::kAppDir), uid, gid) ||
      !ChangeOwnerRecursive(data_dir_.Append(info::kAppDBPath), uid, gid) ||
      !ChangeOwnerRecursive(
          data_dir_.Append(info::kAppDBJournalPath), uid, gid))
    return false;

  if (access(xml_path_.MaybeAsASCII().c_str(), F_OK) != 0)
    return false;
  int result = pkgmgr_parser_parse_manifest_for_installation(
      xml_path_.MaybeAsASCII().c_str(), NULL);
  if (result != 0) {
    LOG(ERROR) << "Manifest parser error: " << result;
    return false;
  }
  LOG(INFO) << "Writing package information in database. [DONE]";
  return true;
}

bool PackageInstaller::Install() {
  return GeneratePkgInfoXml() &&
      CopyOrLinkResources() &&
      WriteToPackageInfoDB();
}

bool PackageInstaller::Uninstall() {
  bool success = true;
  int result = pkgmgr_parser_parse_manifest_for_uninstallation(
      xml_path_.MaybeAsASCII().c_str(), NULL);
  if (result != 0) {
    LOG(ERROR) << "Manifest parser error: " << result;
    return false;
  }

  success &= base::DeleteFile(icon_path_, false);
  success &= base::DeleteFile(execute_path_, false);
  success &= base::DeleteFile(xml_path_, false);
  if (!success)
    return false;

  LOG(INFO) << "Removing and unlinking files from installed locations. [DONE]";
  return true;
}

bool PackageInstaller::ChangeOwnerRecursive(
    const base::FilePath& path,
    const uid_t& uid,
    const gid_t& gid) {
  if (lchown(path.MaybeAsASCII().c_str(), uid, gid) != 0) {
    LOG(ERROR) << "Failed to change ownership of " << path.MaybeAsASCII();
    return false;
  }

  base::FileEnumerator file_iter(
      path,
      true,
      base::FileEnumerator::FILES|base::FileEnumerator::DIRECTORIES);
  base::FilePath file(file_iter.Next());
  while (!file.empty()) {
    if (!ChangeOwnerRecursive(file, uid, gid))
      return false;
    file = file_iter.Next();
  }
  return true;
}

}  // namespace application
}  // namespace xwalk
