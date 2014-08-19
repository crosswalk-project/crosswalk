// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include <glib.h>
#include <gio/gio.h>
#include <locale.h>

#include "base/at_exit.h"
#include "base/memory/scoped_ptr.h"
#include "base/path_service.h"

#include "dbus/bus.h"
#include "dbus/message.h"
#include "dbus/object_proxy.h"

#include "xwalk/application/common/application_storage.h"
#include "xwalk/application/common/installer/package_installer.h"
#include "xwalk/application/tools/linux/dbus_connection.h"
#include "xwalk/runtime/common/xwalk_paths.h"

#if defined(OS_TIZEN)
#include "xwalk/application/common/id_util.h"
#include "xwalk/application/tools/linux/xwalk_tizen_user.h"
#endif

using xwalk::application::ApplicationData;
using xwalk::application::ApplicationStorage;
using xwalk::application::PackageInstaller;

namespace {

char* install_path = NULL;
char* uninstall_id = NULL;
#if defined(OS_TIZEN)
char* operation_key = NULL;
int quiet = 0;
#endif

gint debugging_port = -1;
gboolean continue_tasks = FALSE;

GOptionEntry entries[] = {
  { "install", 'i', 0, G_OPTION_ARG_STRING, &install_path,
    "Path of the application to be installed/updated", "PATH" },
  { "uninstall", 'u', 0, G_OPTION_ARG_STRING, &uninstall_id,
    "Uninstall the application with this appid/pkgid", "ID" },
  { "debugging_port", 'd', 0, G_OPTION_ARG_INT, &debugging_port,
    "Enable remote debugging, port number 0 means to disable", NULL },
  { "continue", 'c' , 0, G_OPTION_ARG_NONE, &continue_tasks,
    "Continue the previous unfinished tasks.", NULL},
#if defined(OS_TIZEN)
  { "key", 'k', 0, G_OPTION_ARG_STRING, &operation_key,
    "Unique operation key", "KEY" },
  { "quiet", 'q', 0, G_OPTION_ARG_NONE, &quiet,
    "Quiet mode", NULL },
#endif
  { NULL }
};

}  // namespace

#if defined(SHARED_PROCESS_MODE)
namespace {

const char xwalk_service_name[] = "org.crosswalkproject.Runtime1";
const char xwalk_running_manager_iface[] =
    "org.crosswalkproject.Running.Manager1";
const dbus::ObjectPath kRunningManagerDBusPath("/running1");

bool enable_remote_debugging(gint debugging_port) {
  dbus::Bus::Options options;
#if defined(OS_TIZEN_MOBILE)
  options.bus_type = dbus::Bus::CUSTOM_ADDRESS;
  options.address.assign("unix:path=/run/user/app/dbus/user_bus_socket");
#endif
  scoped_refptr<dbus::Bus> bus(new dbus::Bus(options));
  dbus::ObjectProxy* app_proxy =
      bus->GetObjectProxy(
          xwalk_service_name,
          kRunningManagerDBusPath);
  if (!app_proxy)
    return false;

  dbus::MethodCall method_call(
      xwalk_running_manager_iface, "EnableRemoteDebugging");
  dbus::MessageWriter writer(&method_call);
  writer.AppendUint32(debugging_port);

  app_proxy->CallMethodAndBlock(&method_call, 1000);

  if (debugging_port > 0) {
    g_print("Remote debugging enabled at port '%d'\n", debugging_port);
  } else {
    g_print("Remote debugging has been disabled\n");
  }
  return true;
}

}  // namespace
#endif

bool list_applications(ApplicationStorage* storage) {
  std::vector<std::string> app_ids;
  if (!storage->GetInstalledApplicationIDs(app_ids))
    return false;

  g_print("Application ID                       Application Name\n");
  g_print("-----------------------------------------------------\n");
  for (unsigned i = 0; i < app_ids.size(); ++i) {
    scoped_refptr<ApplicationData> app_data =
        storage->GetApplicationData(app_ids.at(i));
    if (!app_data) {
      g_print("Failed to obtain app data for xwalk id: %s\n",
              app_ids.at(i).c_str());
      continue;
    }
    g_print("%s  %s\n", app_ids.at(i).c_str(), app_data->Name().c_str());
  }
  g_print("-----------------------------------------------------\n");

  return true;
}

int main(int argc, char* argv[]) {
  setlocale(LC_ALL, "");
  GError* error = NULL;
  GOptionContext* context;
  bool success = false;

#if !GLIB_CHECK_VERSION(2, 36, 0)
  // g_type_init() is deprecated on GLib since 2.36, Tizen has 2.32.
  g_type_init();
#endif

#if defined(OS_TIZEN)
  if (xwalk_tizen_check_user_app())
    exit(1);
#endif

  context = g_option_context_new("- Crosswalk Application Management");
  if (!context) {
    g_print("g_option_context_new failed\n");
    exit(1);
  }
  g_option_context_add_main_entries(context, entries, NULL);
  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    g_print("option parsing failed: %s\n", error->message);
    g_option_context_free(context);
    exit(1);
  }

  g_option_context_free(context);

  base::AtExitManager at_exit;
  base::FilePath data_path;
  xwalk::RegisterPathProvider();
  PathService::Get(xwalk::DIR_DATA_PATH, &data_path);
  scoped_ptr<ApplicationStorage> storage(new ApplicationStorage(data_path));
  scoped_ptr<PackageInstaller> installer =
      PackageInstaller::Create(storage.get());

#if defined(OS_TIZEN)
  installer->SetQuiet(static_cast<bool>(quiet));
  if (operation_key)
    installer->SetInstallationKey(operation_key);
#endif

  if (continue_tasks) {
    g_print("trying to continue previous unfinished tasks.\n");
    installer->ContinueUnfinishedTasks();
    success = true;
    g_print("Previous tasks have been finished.\n");
  }

  if (install_path) {
    std::string app_id;
    const base::FilePath& path = base::FilePath(install_path);
    success = installer->Install(path, &app_id);
    if (!success && storage->Contains(app_id)) {
      g_print("trying to update %s\n", app_id.c_str());
      success = installer->Update(app_id, path);
    }
  } else if (uninstall_id) {
    success = installer->Uninstall(uninstall_id);
  } else if (debugging_port >= 0) {
#if defined(SHARED_PROCESS_MODE)
    // Deal with the case "xwalkctl -d PORT_NUMBER"
    success = enable_remote_debugging(debugging_port);
#else
    g_print("Couldn't enable remote debugging for no shared process mode!");
#endif
  } else if (!continue_tasks) {
    success = list_applications(storage.get());
  }

  return success ? 0 : 1;
}
