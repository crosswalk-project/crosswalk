// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// On Tizen installing applications requires super-user powers primarily because
// some pieces of information of the application are put on globally accessible
// locations. This helper will run with super-user powers (via suid) and will
// be called by Crosswalk (now running as a normal user) so all the activities
// that required 'root' access are done by a small code base.


#if defined(OS_TIZEN_MOBILE)
#include <pkgmgr/pkgmgr_parser.h>
#else
// So we can compile this on Linux Desktop
static int pkgmgr_parser_parse_manifest_for_installation(
    const char* path, char *const tagv[]) {
  return 0;
}

static int pkgmgr_parser_parse_manifest_for_uninstallation(
    const char* path, char *const tagv[]) {
  return 0;
}

#endif

#include "base/files/file_path.h"
#include "base/file_util.h"

namespace {

const base::FilePath kIconDir("/opt/share/icons/default/small/");
const base::FilePath kXmlDir("/opt/share/packages/");
const base::FilePath kXWalkLauncherBinary("/usr/bin/xwalk-launcher");

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

bool InstallApplication(const char* appid, const char* xmlpath,
                        const char* iconpath) {
  base::FilePath icon_src(iconpath);
  // icon_dst == /opt/share/icons/default/small/<appid>.png
  // FIXME(vcgomes): Add support for more icon types
  base::FilePath icon_dst = kIconDir.Append(std::string(appid) + ".png");
  if (!base::CopyFile(icon_src, icon_dst)) {
    fprintf(stdout, "Couldn't copy application icon to '%s'\n",
            icon_dst.value().c_str());
    return false;
  }

  FileDeleter icon_cleaner(icon_dst, false);

  base::FilePath xml_src(xmlpath);
  base::FilePath xml_dst = kXmlDir.Append(std::string(appid) + ".xml");
  if (!base::CopyFile(xml_src, xml_dst)) {
    fprintf(stdout, "Couldn't copy application XML metadata to '%s'\n",
            xml_dst.value().c_str());
    return false;
  }

  FileDeleter xml_cleaner(xml_dst, false);

  if (pkgmgr_parser_parse_manifest_for_installation(xmlpath, NULL)) {
    fprintf(stdout, "Couldn't parse manifest XML '%s'\n", xmlpath);
    return false;
  }

  icon_cleaner.Dismiss();
  xml_cleaner.Dismiss();

  return true;
}

bool UninstallApplication(const char* appid) {
  bool result = true;
  char iconname[PATH_MAX];

  // FIXME(vcgomes): Add support for more icon types
  base::FilePath icon_dst = kIconDir.Append(std::string(appid) + ".png");
  if (!base::DeleteFile(icon_dst, false)) {
    fprintf(stdout, "Couldn't delete '%s'\n", icon_dst.value().c_str());
    result = false;
  }

  base::FilePath xmlpath(kXmlDir);
  xmlpath = xmlpath.Append(std::string(appid) + ".xml");

  int ret = pkgmgr_parser_parse_manifest_for_uninstallation(
      xmlpath.value().c_str(), NULL);
  if (ret) {
    fprintf(stdout, "Couldn't parse manifest XML '%s'\n",
            xmlpath.value().c_str());
    result = false;
  }

  if (!base::DeleteFile(xmlpath, false)) {
    fprintf(stdout, "Couldn't delete '%s'\n", xmlpath.value().c_str());
    result = false;
  }

  return result;
}

int usage(const char* program) {
  fprintf(stdout, "%s - Crosswalk Tizen Application Installation helper\n\n",
          basename(program));
  fprintf(stdout, "Usage: \n"
          "\t%s --install <appid> <xml> <icon>\n"
          "\t%s --uninstall <appid>\n",
          program, program);
  return 1;
}

}  // namespace

int main(int argc, char *argv[]) {
  bool result = false;

  if (argc <= 2)
    return usage(argv[0]);

  if (!strcmp(argv[1], "--install")) {
    if (argc != 5)
      return usage(argv[0]);

    result = InstallApplication(argv[2], argv[3], argv[4]);
  } else if (!strcmp(argv[1], "--uninstall")) {
    if (argc != 3)
      return usage(argv[0]);

    result = UninstallApplication(argv[2]);
  } else {
    return usage(argv[0]);
  }

  // Convetion is to return 0 on success.
  return result ? 0 : 1;
}
