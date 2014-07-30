// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/installer/package_installer_tizen.h"

#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <pkgmgr/pkgmgr_parser.h>
#include <algorithm>
#include <map>
#include <string>

#include "base/file_util.h"
#include "base/files/file_enumerator.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/command_line.h"
#include "base/process/launch.h"
#include "third_party/libxml/chromium/libxml_utils.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/manifest_handlers/tizen_application_handler.h"
#include "xwalk/application/common/manifest_handlers/tizen_metadata_handler.h"
#include "xwalk/application/common/application_storage.h"
#include "xwalk/application/common/installer/tizen/packageinfo_constants.h"
#include "xwalk/application/common/id_util.h"
#include "xwalk/runtime/common/xwalk_paths.h"

namespace info = xwalk::application_packageinfo_constants;

namespace {

namespace widget_keys = xwalk::application_widget_keys;

const base::FilePath kPkgHelper("/usr/bin/xwalk-pkg-helper");

const base::FilePath kXWalkLauncherBinary("/usr/bin/xwalk-launcher");

const base::FilePath kDefaultIcon(
    "/usr/share/icons/default/small/crosswalk.png");

const std::string kServicePrefix("xwalk-service.");
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

void WriteMetaDataElement(
    XmlWriter& writer, // NOLINT
    xwalk::application::TizenMetaDataInfo* info) {
  if (!info)
    return;

  const std::map<std::string, std::string>& metadata = info->metadata();
  std::map<std::string, std::string>::const_iterator it;
  for (it = metadata.begin(); it != metadata.end(); ++it) {
    writer.StartElement("metadata");
    writer.AddAttribute("key", it->first);
    writer.AddAttribute("value", it->second);
    writer.EndElement();
  }
}

// For xpk, package_id => [crosswalk_32bytes_app_id]
// For wgt, package_id => [tizen_wrt_10bytes_package_id]
std::string GetTizenPackageId(
    xwalk::application::ApplicationData* application) {
  if (application->GetPackageType() == xwalk::application::Package::XPK)
    return application->ID();

  const xwalk::application::TizenApplicationInfo* tizen_app_info =
      static_cast<xwalk::application::TizenApplicationInfo*>(
          application->GetManifestData(widget_keys::kTizenApplicationKey));
  CHECK(tizen_app_info);
  return tizen_app_info->package();
}

bool GeneratePkgInfoXml(xwalk::application::ApplicationData* application,
                        const std::string& icon_name,
                        const base::FilePath& app_dir,
                        const base::FilePath& xml_path) {
  if (!base::PathExists(app_dir) &&
      !base::CreateDirectory(app_dir))
    return false;

  std::string package_id = GetTizenPackageId(application);
  std::string tizen_app_id = GetTizenAppId(application);

  base::FilePath execute_path =
      app_dir.AppendASCII("bin/").AppendASCII(tizen_app_id);
  std::string stripped_name = application->Name();

  FILE* file = base::OpenFile(xml_path, "w");

  XmlWriter xml_writer;
  xml_writer.StartWriting();
  xml_writer.StartElement("manifest");
  xml_writer.AddAttribute("xmlns", "http://tizen.org/ns/packages");
  xml_writer.AddAttribute("package", package_id);
  xml_writer.AddAttribute("type", "wgt");
  xml_writer.AddAttribute("version", application->VersionString());
  xml_writer.WriteElement("label", application->Name());
  xml_writer.WriteElement("description", application->Description());

  xml_writer.StartElement("ui-application");
  xml_writer.AddAttribute("appid", tizen_app_id);
  xml_writer.AddAttribute("exec", execute_path.MaybeAsASCII());
  xml_writer.AddAttribute("type", "webapp");
  xml_writer.AddAttribute("taskmanage", "true");
  xml_writer.WriteElement("label", application->Name());

  xwalk::application::TizenMetaDataInfo* info =
      static_cast<xwalk::application::TizenMetaDataInfo*>(
      application->GetManifestData(widget_keys::kTizenMetaDataKey));
  WriteMetaDataElement(xml_writer, info);

  if (icon_name.empty())
    xml_writer.WriteElement("icon", info::kDefaultIconName);
  else
    xml_writer.WriteElement("icon", kServicePrefix + package_id + ".png");
  xml_writer.EndElement();  // Ends "ui-application"

  xml_writer.EndElement();  // Ends "manifest" element.
  xml_writer.StopWriting();

  base::WriteFile(xml_path,
                  xml_writer.GetWrittenString().c_str(),
                  xml_writer.GetWrittenString().size());

  base::CloseFile(file);
  LOG(INFO) << "Converting manifest.json into "
            << xml_path.BaseName().MaybeAsASCII()
            << " for installation. [DONE]";
  return true;
}

bool CreateAppSymbolicLink(const base::FilePath& app_dir,
                           const std::string& tizen_app_id) {
  base::FilePath execute_path =
      app_dir.AppendASCII("bin/").AppendASCII(tizen_app_id);

  if (!base::CreateDirectory(execute_path.DirName())) {
    LOG(ERROR) << "Could not create directory '"
               << execute_path.DirName().value() << "'.";
    return false;
  }

  if (!base::CreateSymbolicLink(kXWalkLauncherBinary, execute_path)) {
    LOG(ERROR) << "Could not create symbolic link to launcher from '"
               << execute_path.value() << "'.";
    return false;
  }
  return true;
}

}  // namespace

namespace xwalk {
namespace application {

PackageInstallerTizen::PackageInstallerTizen(ApplicationStorage* storage)
    : PackageInstaller(storage) {
}

bool PackageInstallerTizen::PlatformInstall(ApplicationData* app_data) {
  std::string app_id(app_data->ID());
  base::FilePath data_dir;
  CHECK(PathService::Get(xwalk::DIR_DATA_PATH, &data_dir));

  std::string tizen_app_id = GetTizenAppId(app_data);
  base::FilePath app_dir =
      data_dir.AppendASCII(info::kAppDir).AppendASCII(app_id);
  base::FilePath xml_path = data_dir.AppendASCII(info::kAppDir).AppendASCII(
      app_id + std::string(info::kXmlExtension));

  std::string icon_name;
  if (!app_data->GetManifest()->GetString(info::kIconKey, &icon_name)) {
    LOG(WARNING) << "'icon' not included in manifest";
  }
  // This will clean everything inside '<data dir>/<app id>'.
  FileDeleter app_dir_cleaner(app_dir, true);

  if (!GeneratePkgInfoXml(app_data, icon_name, app_dir, xml_path)) {
    LOG(ERROR) << "Failed to create XML metadata file '"
               << xml_path.value() << "'.";
    return false;
  }

  if (!CreateAppSymbolicLink(app_dir, tizen_app_id)) {
    LOG(ERROR) << "Failed to create symbolic link for " << app_id;
    return false;
  }

  base::FilePath icon =
      icon_name.empty() ? kDefaultIcon : app_dir.AppendASCII(icon_name);

  CommandLine cmdline(kPkgHelper);
  cmdline.AppendSwitch("--install");
  cmdline.AppendArg(app_id);
  cmdline.AppendArgPath(xml_path);
  cmdline.AppendArgPath(icon);

  int exit_code;
  std::string output;

  if (!base::GetAppOutputWithExitCode(cmdline, &output, &exit_code)) {
    LOG(ERROR) << "Could not launch the installation helper process.";
    return false;
  }

  if (exit_code != 0) {
    LOG(ERROR) << "Could not install application: "
               << output << " (" << exit_code << ")";
    return false;
  }

  app_dir_cleaner.Dismiss();

  return true;
}

bool PackageInstallerTizen::PlatformUninstall(ApplicationData* app_data) {
  bool result = true;
  std::string app_id(app_data->ID());
  base::FilePath data_dir;
  CHECK(PathService::Get(xwalk::DIR_DATA_PATH, &data_dir));

  CommandLine cmdline(kPkgHelper);
  cmdline.AppendSwitch("--uninstall");
  cmdline.AppendArg(app_id);

  int exit_code;
  std::string output;

  if (!base::GetAppOutputWithExitCode(cmdline, &output, &exit_code)) {
    LOG(ERROR) << "Could not launch installer helper";
    result = false;
  }

  if (exit_code != 0) {
    LOG(ERROR) << "Could not uninstall application: "
               << output << " (" << exit_code << ")";
    result = false;
  }

  base::FilePath app_dir =
      data_dir.AppendASCII(info::kAppDir).AppendASCII(app_id);
  if (!base::DeleteFile(app_dir, true)) {
    LOG(ERROR) << "Could not remove directory '" << app_dir.value() << "'";
    result = false;
  }

  base::FilePath xml_path = data_dir.AppendASCII(
      app_id + std::string(info::kXmlExtension));
  if (!base::DeleteFile(xml_path, false)) {
    LOG(ERROR) << "Could not remove file '" << xml_path.value() << "'";
    result = false;
  }

  return result;
}

bool PackageInstallerTizen::PlatformUpdate(ApplicationData* app_data) {
  std::string app_id(app_data->ID());
  base::FilePath data_dir;
  CHECK(PathService::Get(xwalk::DIR_DATA_PATH, &data_dir));

  std::string tizen_app_id = GetTizenAppId(app_data);
  base::FilePath app_dir =
      data_dir.AppendASCII(info::kAppDir).AppendASCII(app_id);
  base::FilePath new_xml_path = data_dir.AppendASCII(info::kAppDir).AppendASCII(
      app_id + ".new" + std::string(info::kXmlExtension));

  std::string icon_name;
  if (!app_data->GetManifest()->GetString(info::kIconKey, &icon_name)) {
    LOG(WARNING) << "'icon' not included in manifest";
  }
  // This will clean everything inside '<data dir>/<app id>' and the new XML.
  FileDeleter app_dir_cleaner(app_dir, true);
  FileDeleter new_xml_cleaner(new_xml_path, true);

  if (!GeneratePkgInfoXml(app_data, icon_name, app_dir, new_xml_path)) {
    LOG(ERROR) << "Could not create new XML metadata file '"
               << new_xml_path.value() << "'.";
    return false;
  }

  if (!CreateAppSymbolicLink(app_dir, tizen_app_id))
    return false;

  base::FilePath icon =
      icon_name.empty() ? kDefaultIcon : app_dir.AppendASCII(icon_name);

  CommandLine cmdline(kPkgHelper);
  cmdline.AppendSwitch("--update");
  cmdline.AppendArg(app_id);
  cmdline.AppendArgPath(new_xml_path);
  cmdline.AppendArgPath(icon);

  int exit_code;
  std::string output;

  if (!base::GetAppOutputWithExitCode(cmdline, &output, &exit_code)) {
    LOG(ERROR) << "Could not launch installer helper";
    return false;
  }

  if (exit_code != 0) {
    LOG(ERROR) << "Could not update application: "
               << output << " (" << exit_code << ")";
    return false;
  }

  base::FilePath old_xml_path = data_dir.AppendASCII(info::kAppDir).AppendASCII(
      app_id + std::string(info::kXmlExtension));
  base::Move(new_xml_path, old_xml_path);
  app_dir_cleaner.Dismiss();
  return true;
}

}  // namespace application
}  // namespace xwalk
