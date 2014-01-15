// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/installer/tizen/service_package_installer.h"

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
#include "base/command_line.h"
#include "base/process/launch.h"
#include "third_party/libxml/chromium/libxml_utils.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/browser/application_storage.h"
#include "xwalk/application/browser/installer/tizen/packageinfo_constants.h"

namespace info = xwalk::application_packageinfo_constants;

namespace {

const base::FilePath kPkgHelper("/usr/bin/xwalk-pkg-helper");

const base::FilePath kXWalkLauncherBinary("/usr/bin/xwalk-launcher");

const base::FilePath kDefaultIcon(
    "/usr/share/icons/default/small/crosswalk.png");

const std::string kServicePrefix("xwalk-service.");

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

bool GeneratePkgInfoXml(xwalk::application::ApplicationData* application,
                        const std::string& icon_name,
                        const base::FilePath& app_dir,
                        const base::FilePath& xml_path) {
  if (!base::PathExists(app_dir) &&
      !file_util::CreateDirectory(app_dir))
    return false;

  std::string package_id = application->ID();
  base::FilePath execute_path =
      app_dir.AppendASCII("bin/").AppendASCII(package_id);
  std::string stripped_name = application->Name();
  stripped_name.erase(
      std::remove_if(stripped_name.begin(), stripped_name.end(), ::isspace),
      stripped_name.end());

  FILE* file = file_util::OpenFile(xml_path, "w");

  XmlWriter xml_writer;
  xml_writer.StartWriting();
  xml_writer.StartElement("manifest");
  xml_writer.AddAttribute("xmlns", "http://tizen.org/ns/packages");
  xml_writer.AddAttribute("package", package_id);
  xml_writer.AddAttribute("version", application->VersionString());
  xml_writer.WriteElement("label", application->Name());
  xml_writer.WriteElement("description", application->Description());

  xml_writer.StartElement("ui-application");
  xml_writer.AddAttribute(
      "appid", kServicePrefix + package_id + info::kSeparator + stripped_name);
  xml_writer.AddAttribute("exec", execute_path.MaybeAsASCII());
  xml_writer.AddAttribute("type", "c++app");
  xml_writer.AddAttribute("taskmanage", "true");
  xml_writer.WriteElement("label", application->Name());

  if (icon_name.empty())
    xml_writer.WriteElement("icon", info::kDefaultIconName);
  else
    xml_writer.WriteElement("icon", kServicePrefix + package_id + ".png");
  xml_writer.EndElement();  // Ends "ui-application"

  xml_writer.EndElement();  // Ends "manifest" element.
  xml_writer.StopWriting();

  file_util::WriteFile(xml_path,
                       xml_writer.GetWrittenString().c_str(),
                       xml_writer.GetWrittenString().size());

  file_util::CloseFile(file);
  LOG(INFO) << "Converting manifest.json into "
            << xml_path.BaseName().MaybeAsASCII()
            << " for installation. [DONE]";
  return true;
}

}  // namespace

namespace xwalk {
namespace application {

bool InstallApplicationForTizen(
    ApplicationData* application, const base::FilePath& data_dir) {
  std::string package_id = application->ID();
  base::FilePath app_dir =
      data_dir.AppendASCII(info::kAppDir).AppendASCII(package_id);
  base::FilePath xml_path = data_dir.AppendASCII(info::kAppDir).AppendASCII(
      package_id + std::string(info::kXmlExtension));

  std::string icon_name;
  if (!application->GetManifest()->GetString(info::kIconKey, &icon_name)) {
    LOG(WARNING) << "'icon' not included in manifest";
  }
  // This will clean everything inside '<data dir>/<app id>'.
  FileDeleter app_dir_cleaner(app_dir, true);

  if (!GeneratePkgInfoXml(application, icon_name, app_dir, xml_path)) {
    LOG(ERROR) << "Could not create XML metadata file '"
               << xml_path.value() << "'.";
    return false;
  }

  base::FilePath execute_path =
      app_dir.AppendASCII("bin/").AppendASCII(package_id);

  if (!file_util::CreateDirectory(execute_path.DirName())) {
    LOG(ERROR) << "Could not create directory '"
               << execute_path.DirName().value() << "'.";
    return false;
  }

  if (!file_util::CreateSymbolicLink(kXWalkLauncherBinary, execute_path)) {
    LOG(ERROR) << "Could not create symbolic link to launcher from '"
               << execute_path.value() << "'.";
    return false;
  }

  base::FilePath icon;
  if (icon_name.empty())
    icon = kDefaultIcon;
  else
    icon = app_dir.AppendASCII(icon_name);

  CommandLine cmdline(kPkgHelper);
  cmdline.AppendSwitch("--install");
  cmdline.AppendArg(package_id);
  cmdline.AppendArgPath(xml_path);
  cmdline.AppendArgPath(icon);

  int exit_code;
  std::string output;

  if (!base::GetAppOutputWithExitCode(cmdline, &output, &exit_code)) {
    LOG(ERROR) << "Could not launch installer helper.";
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

bool UninstallApplicationForTizen(ApplicationData* application,
                                  const base::FilePath& data_dir) {
  std::string package_id = application->ID();
  bool result = true;

  CommandLine cmdline(kPkgHelper);
  cmdline.AppendSwitch("--uninstall");
  cmdline.AppendArg(package_id);

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
      data_dir.AppendASCII(info::kAppDir).AppendASCII(package_id);
  if (!base::DeleteFile(app_dir, true)) {
    LOG(ERROR) << "Could not remove directory '" << app_dir.value() << "'";
    result = false;
  }

  base::FilePath xml_path = data_dir.AppendASCII(
      package_id + std::string(info::kXmlExtension));
  if (!base::DeleteFile(xml_path, false)) {
    LOG(ERROR) << "Could not remove file '" << xml_path.value() << "'";
    result = false;
  }

  return result;
}

}  // namespace application
}  // namespace xwalk
