// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <gio/gio.h>
#include <glib.h>

#include "base/at_exit.h"
#include "base/files/file_util.h"
#include "base/memory/scoped_ptr.h"
#include "base/path_service.h"
#include "xwalk/application/common/id_util.h"
#include "xwalk/application/common/tizen/application_storage.h"
#include "xwalk/application/tools/linux/dbus_connection.h"
#include "xwalk/application/tools/tizen/xwalk_package_installer.h"
#include "xwalk/application/tools/tizen/xwalk_tizen_user.h"
#include "xwalk/runtime/common/xwalk_paths.h"

using xwalk::application::ApplicationData;
using xwalk::application::ApplicationStorage;

namespace {

char* install_path = NULL;
char* uninstall_id = NULL;
char* reinstall_id = NULL;
char* operation_key = NULL;
int quiet = 0;

gboolean continue_tasks = FALSE;

GOptionEntry entries[] = {
  { "install", 'i', 0, G_OPTION_ARG_STRING, &install_path,
    "Path of the application to be installed/updated", "PATH" },
  { "uninstall", 'd', 0, G_OPTION_ARG_STRING, &uninstall_id,
    "Uninstall the application with this pkgid", "ID" },
  { "continue", 'c' , 0, G_OPTION_ARG_NONE, &continue_tasks,
    "Continue the previous unfinished tasks", NULL},
  { "reinstall", 'r', 0, G_OPTION_ARG_STRING, &reinstall_id,
    "Reinstall the application with this pkgid "
    "(This option is ONLY for SDK to support RDS mode"
    " (Rapid Development Support)", "ID" },
  { "key", 'k', 0, G_OPTION_ARG_STRING, &operation_key,
    "Unique operation key", "KEY" },
  { "quiet", 'q', 0, G_OPTION_ARG_NONE, &quiet,
    "Quiet mode", NULL },
  { NULL }
};

}  // namespace

int main(int argc, char* argv[]) {
  GError* error = NULL;
  GOptionContext* context;
  bool success = false;

#if !GLIB_CHECK_VERSION(2, 36, 0)
  // g_type_init() is deprecated on GLib since 2.36, Tizen has 2.32.
  g_type_init();
#endif

  if (xwalk_tizen_check_user_for_xwalk_backend())
    return 1;

  context = g_option_context_new("- Crosswalk Application Management");
  if (!context) {
    g_print("g_option_context_new failed\n");
    return 1;
  }
  g_option_context_add_main_entries(context, entries, NULL);
  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    g_print("option parsing failed: %s\n", error->message);
    g_option_context_free(context);
    return 1;
  }

  g_option_context_free(context);

  base::AtExitManager at_exit;
  base::FilePath data_path;
  xwalk::RegisterPathProvider();
  PathService::Get(xwalk::DIR_DATA_PATH, &data_path);
  scoped_ptr<ApplicationStorage> storage(new ApplicationStorage(data_path));
  scoped_ptr<PackageInstaller> installer =
      PackageInstaller::Create(storage.get());

  installer->SetQuiet(static_cast<bool>(quiet));
  if (operation_key)
    installer->SetInstallationKey(operation_key);

  if (continue_tasks) {
    g_print("trying to continue previous unfinished tasks.\n");
    installer->ContinueUnfinishedTasks();
    success = true;
    g_print("Previous tasks have been finished.\n");
  }

  if (install_path) {
    std::string app_id;
    const base::FilePath& path =
        base::MakeAbsoluteFilePath(base::FilePath(install_path));
    success = installer->Install(path, &app_id);
    if (!success && !app_id.empty() && storage->Contains(app_id)) {
      g_print("trying to update %s\n", app_id.c_str());
      success = installer->Update(app_id, path);
    }
  } else if (uninstall_id) {
    success = installer->Uninstall(uninstall_id);
  } else if (reinstall_id) {
    success = installer->Reinstall(reinstall_id);
  }
  return success ? 0 : 1;
}
