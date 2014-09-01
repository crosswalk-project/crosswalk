// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// On Tizen installing applications requires super-user powers primarily because
// some pieces of information of the application are put on globally accessible
// locations. This helper will run with super-user powers (via suid) and will
// be called by Crosswalk (now running as a normal user) so all the activities
// that required 'root' access are done by a small code base.

#include <glib.h>
#include <gio/gio.h>

#include <string>

#include "base/files/file_path.h"
#include "base/file_util.h"
#include "xwalk/application/tools/tizen/xwalk_package_installer_helper.h"

namespace {

char* install_option = NULL;
char* update_option = NULL;
char* uninstall_option = NULL;
char* reinstall_option = NULL;
char* operation_key = NULL;
char* xml_path = NULL;
char* icon_path = NULL;
int quiet = 0;

GOptionEntry entries[] = {
  { "install", 'i', 0, G_OPTION_ARG_STRING, &install_option,
    "Path of the application to be installed", "APPID" },
  { "update", 'u', 0, G_OPTION_ARG_STRING, &update_option,
    "Path of the application to be updated", "APPID" },
  { "uninstall", 'd', 0, G_OPTION_ARG_STRING, &uninstall_option,
    "Uninstall the application with this appid", "APPID" },
  { "reinstall", 'r', 0, G_OPTION_ARG_STRING, &reinstall_option,
    "Path of the application to be reinstalled", "APPID" },
  { "key", 'k', 0, G_OPTION_ARG_STRING, &operation_key,
    "Unique operation key", "KEY" },
  { "quiet", 'q', 0, G_OPTION_ARG_NONE, &quiet,
    "Quiet mode", NULL },
  { "xml", 'x', 0, G_OPTION_ARG_STRING, &xml_path,
    "Xml file", "XML_FILE" },
  { "icon", 'y', 0, G_OPTION_ARG_STRING, &icon_path,
    "Icon file", "ICON_FILE" },
  { NULL }
};

}  // namespace

int main(int argc, char *argv[]) {
  GError* error = NULL;
  GOptionContext* context;

#if !GLIB_CHECK_VERSION(2, 36, 0)
  // g_type_init() is deprecated on GLib since 2.36, Tizen has 2.32.
  g_type_init();
#endif

  context = g_option_context_new(
        " - Crosswalk Tizen Application Installation helper");
  if (!context) {
    fprintf(stderr, "g_option_context_new failed\n");
    exit(1);
  }
  g_option_context_add_main_entries(context, entries, NULL);
  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    fprintf(stderr, "option parsing failed: %s\n", error->message);
    g_option_context_free(context);
    exit(1);
  }
  g_option_context_free(context);

  bool result = false;

  // args for pkgmgr
  const char* pkgmgr_argv[5];
  pkgmgr_argv[2] = "-k";
  pkgmgr_argv[3] = operation_key;
  pkgmgr_argv[4] = "-q";

  char* appId = NULL;
  if (install_option) {
    appId = install_option;
  } else if (uninstall_option) {
    appId = uninstall_option;
  } else if (update_option) {
    appId = update_option;
  } else if (reinstall_option) {
    appId = reinstall_option;
  } else {
    fprintf(stderr, "Use: --install, --uninstall, --update or --reinstall\n");
    exit(1);
  }

  PackageInstallerHelper helper(appId);

  if (install_option) {
    if (!xml_path || !icon_path) {
      fprintf(stdout, "missing --xml or --icon option\n");
      exit(1);
    }

     if (operation_key) {
      pkgmgr_argv[0] = "-i";
      pkgmgr_argv[1] = install_option;  // this value is ignored by pkgmgr
      helper.InitializePkgmgrSignal((quiet ? 5 : 4), pkgmgr_argv);
    }

    result = helper.InstallApplication(xml_path, icon_path);
  } else if (uninstall_option) {
    if (operation_key) {
      pkgmgr_argv[0] = "-d";
      pkgmgr_argv[1] = uninstall_option;  // this value is ignored by pkgmgr
      helper.InitializePkgmgrSignal((quiet ? 5 : 4), pkgmgr_argv);
    }

    result = helper.UninstallApplication();
  } else if (update_option) {
    if (!xml_path || !icon_path) {
      fprintf(stderr, "missing --xml or --icon option\n");
      exit(1);
    }

    if (operation_key) {
      pkgmgr_argv[0] = "-i";
      pkgmgr_argv[1] = update_option;  // this value is ignored by pkgmgr
      helper.InitializePkgmgrSignal((quiet ? 5 : 4), pkgmgr_argv);
    }

    result = helper.UpdateApplication(xml_path, icon_path);
  } else if (reinstall_option) {
    if (operation_key) {
      pkgmgr_argv[0] = "-r";
      pkgmgr_argv[1] = reinstall_option;  // this value is ignored by pkgmgr
      helper.InitializePkgmgrSignal((quiet ? 5 : 4), pkgmgr_argv);
    }

    result = helper.ReinstallApplication();
  }

  // Convention is to return 0 on success.
  return result ? 0 : 1;
}
