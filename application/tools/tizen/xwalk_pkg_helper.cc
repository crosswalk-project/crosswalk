// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// On Tizen installing applications requires super-user powers primarily because
// some pieces of information of the application are put on globally accessible
// locations. This helper will run with super-user powers (via suid) and will
// be called by Crosswalk (now running as a normal user) so all the activities
// that required 'root' access are done by a small code base.

#include "base/files/file_path.h"
#include "base/file_util.h"
#include "xwalk/application/tools/tizen/xwalk_pkg_installer.h"

namespace {

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

  // When installing an application on Tizen, the libraries used require
  // some steps to be run as root (UID 0) and fail otherwise, so we force
  // this tool to assume the root UID.
  if (setuid(0)) {
    fprintf(stderr, "Make sure '%s' is set-user-ID-root\n", argv[0]);
    return 1;;
  }

  PkgInstaller installer(argv[2]);
  if (!strcmp(argv[1], "--install")) {
    if (argc != 5)
      return usage(argv[0]);

    result = installer.InstallApplication(argv[3], argv[4]);
  } else if (!strcmp(argv[1], "--uninstall")) {
    if (argc != 3)
      return usage(argv[0]);

    result = installer.UninstallApplication();
  } else {
    return usage(argv[0]);
  }

  // Convetion is to return 0 on success.
  return result ? 0 : 1;
}
